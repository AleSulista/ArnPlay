// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Alessandro Henriques Teixeira — Studio Arn

#include "MpvItem.h"

#include <QMetaObject>
#include <QDebug>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFramebufferObjectFormat>
#include <QQuickWindow>
#include <QStringList>
#include <QFileInfo>

#include <clocale>

#include <mpv/client.h>
#include <mpv/render_gl.h>

namespace {
void *getProcAddress(void *, const char *name)
{
    auto *context = QOpenGLContext::currentContext();
    return context ? reinterpret_cast<void *>(context->getProcAddress(name)) : nullptr;
}
}

class MpvRenderer final : public QQuickFramebufferObject::Renderer
{
public:
    explicit MpvRenderer(MpvItem *item) : m_item(item) {}

    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override
    {
        QOpenGLFramebufferObjectFormat format;
        format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
        return new QOpenGLFramebufferObject(size, format);
    }

    void render() override
    {
        if (!m_item->m_mpv)
            return;

        if (!m_item->m_renderContext) {
            mpv_opengl_init_params glInit{getProcAddress, nullptr};
            mpv_render_param params[] = {
                {MPV_RENDER_PARAM_API_TYPE, const_cast<char *>(MPV_RENDER_API_TYPE_OPENGL)},
                {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &glInit},
                {MPV_RENDER_PARAM_INVALID, nullptr}
            };
            QMutexLocker locker(&m_item->m_renderMutex);
            if (mpv_render_context_create(&m_item->m_renderContext, m_item->m_mpv, params) < 0)
                return;
            mpv_render_context_set_update_callback(m_item->m_renderContext,
                                                   MpvItem::renderUpdate, m_item);
        }

        QOpenGLFramebufferObject *fbo = framebufferObject();
        mpv_opengl_fbo mpvFbo{static_cast<int>(fbo->handle()), fbo->width(), fbo->height(), 0};
        int flipY = 0;
        mpv_render_param params[] = {
            {MPV_RENDER_PARAM_OPENGL_FBO, &mpvFbo},
            {MPV_RENDER_PARAM_FLIP_Y, &flipY},
            {MPV_RENDER_PARAM_INVALID, nullptr}
        };
        QMutexLocker locker(&m_item->m_renderMutex);
        mpv_render_context_render(m_item->m_renderContext, params);
        update();
    }

private:
    MpvItem *m_item;
};

MpvItem::MpvItem(QQuickItem *parent) : QQuickFramebufferObject(parent)
{
    // libmpv requires the numeric locale to use a dot as decimal separator.
    std::setlocale(LC_NUMERIC, "C");
    m_mpv = mpv_create();
    if (!m_mpv) {
        QMetaObject::invokeMethod(this, [this] { emit errorOccurred("Não foi possível iniciar a libmpv."); }, Qt::QueuedConnection);
        return;
    }

    // Keep rendering inside Qt Quick and avoid Vulkan/MoltenVK on older Intel Macs.
    mpv_set_option_string(m_mpv, "vo", "libmpv");
    mpv_set_option_string(m_mpv, "gpu-api", "opengl");
    // Allow mpv's built-in ytdl hook to resolve supported web media URLs.
    // yt-dlp remains an optional external dependency; ArnPlay does not download files.
    mpv_set_option_string(m_mpv, "ytdl", "yes");
    mpv_set_option_string(m_mpv, "volume-max", "200");
    mpv_set_option_string(m_mpv, "keep-open", "yes");
    mpv_set_option_string(m_mpv, "hwdec", "auto-safe");
    const int initializeResult = mpv_initialize(m_mpv);
    if (initializeResult < 0) {
        qCritical() << "Falha ao iniciar libmpv:" << mpv_error_string(initializeResult);
        mpv_terminate_destroy(m_mpv);
        m_mpv = nullptr;
        return;
    }

    mpv_observe_property(m_mpv, 0, "pause", MPV_FORMAT_FLAG);
    mpv_observe_property(m_mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);
    mpv_observe_property(m_mpv, 0, "duration", MPV_FORMAT_DOUBLE);
    mpv_observe_property(m_mpv, 0, "volume", MPV_FORMAT_DOUBLE);
    mpv_observe_property(m_mpv, 0, "media-title", MPV_FORMAT_STRING);
    mpv_set_wakeup_callback(m_mpv, wakeup, this);
}

MpvItem::~MpvItem()
{
    QMutexLocker locker(&m_renderMutex);
    if (m_renderContext)
        mpv_render_context_free(m_renderContext);
    if (m_mpv)
        mpv_terminate_destroy(m_mpv);
}

QQuickFramebufferObject::Renderer *MpvItem::createRenderer() const
{
    return new MpvRenderer(const_cast<MpvItem *>(this));
}

void MpvItem::open(const QUrl &url)
{
    if (!m_mpv || !url.isValid()) return;
    if (!m_loading) {
        m_loading = true;
        emit loadingChanged();
    }
    const QByteArray path = url.isLocalFile() ? url.toLocalFile().toUtf8() : url.toString().toUtf8();
    static const QStringList audioExtensions{
        "mp3", "flac", "wav", "m4a", "aac", "ogg", "oga", "opus", "wma", "aiff", "aif", "alac", "ape", "ac3", "dts"
    };
    m_currentMediaIsAudio = url.isLocalFile() && audioExtensions.contains(QFileInfo(url.toLocalFile()).suffix().toLower());
    mpv_set_property_string(m_mpv, "lavfi-complex", "");
    const char *command[] = {"loadfile", path.constData(), nullptr};
    mpv_command_async(m_mpv, 0, command);
}

void MpvItem::togglePause() { if (m_mpv) setFlag("pause", m_playing); }
void MpvItem::stop() { if (m_mpv) { const char *c[] = {"stop", nullptr}; mpv_command_async(m_mpv, 0, c); } }
void MpvItem::seek(double seconds) { if (m_mpv) { const QByteArray s = QByteArray::number(seconds); const char *c[] = {"seek", s.constData(), "relative", nullptr}; mpv_command_async(m_mpv, 0, c); } }
void MpvItem::addSubtitle(const QUrl &url)
{
    if (!m_mpv || !url.isValid()) return;
    const QByteArray path = url.isLocalFile() ? url.toLocalFile().toUtf8() : url.toString().toUtf8();
    const char *command[] = {"sub-add", path.constData(), "select", nullptr};
    mpv_command_async(m_mpv, 0, command);
}
void MpvItem::cycleAudioTrack()
{
    if (!m_mpv) return;
    const char *command[] = {"cycle", "aid", nullptr};
    mpv_command_async(m_mpv, 0, command);
}
void MpvItem::cycleSubtitleTrack()
{
    if (!m_mpv) return;
    const char *command[] = {"cycle", "sid", nullptr};
    mpv_command_async(m_mpv, 0, command);
}
void MpvItem::setPlaybackSpeed(double speed)
{
    if (m_mpv) mpv_set_property_async(m_mpv, 0, "speed", MPV_FORMAT_DOUBLE, &speed);
}
void MpvItem::setVideoAdjustments(int brightness, int contrast, int saturation, int gamma, int rotation)
{
    if (!m_mpv) return;
    int64_t brightnessValue = brightness;
    int64_t contrastValue = contrast;
    int64_t saturationValue = saturation;
    int64_t gammaValue = gamma;
    mpv_set_property(m_mpv, "brightness", MPV_FORMAT_INT64, &brightnessValue);
    mpv_set_property(m_mpv, "contrast", MPV_FORMAT_INT64, &contrastValue);
    mpv_set_property(m_mpv, "saturation", MPV_FORMAT_INT64, &saturationValue);
    mpv_set_property(m_mpv, "gamma", MPV_FORMAT_INT64, &gammaValue);
    int64_t rotate = rotation;
    mpv_set_property(m_mpv, "video-rotate", MPV_FORMAT_INT64, &rotate);
}
void MpvItem::resetVideoAdjustments() { setVideoAdjustments(0, 0, 0, 0, 0); }
void MpvItem::setAudioAdjustments(int bass, int treble, bool normalize, double delay)
{
    if (!m_mpv) return;
    QStringList filters;
    if (normalize) filters << QStringLiteral("lavfi=[loudnorm]");
    if (bass != 0) filters << QStringLiteral("lavfi=[equalizer=f=100:t=q:w=1:g=%1]").arg(bass);
    if (treble != 0) filters << QStringLiteral("lavfi=[equalizer=f=8000:t=q:w=1:g=%1]").arg(treble);
    const QByteArray af = filters.join(',').toUtf8();
    mpv_set_property_string(m_mpv, "af", af.constData());
    mpv_set_property(m_mpv, "audio-delay", MPV_FORMAT_DOUBLE, &delay);
}
void MpvItem::resetAudioAdjustments() { setAudioAdjustments(0, 0, false, 0.0); }
void MpvItem::setMusicVisualizer(bool enabled, int bands)
{
    m_musicVisualizer = enabled;
    m_visualizerBands = qBound(48, bands, 96);
    configureMusicVisualizer();
}

void MpvItem::configureMusicVisualizer()
{
    if (!m_mpv) return;
    if (!m_musicVisualizer || !m_currentMediaIsAudio) {
        mpv_set_property_string(m_mpv, "lavfi-complex", "");
        return;
    }
    int64_t audioId = 0;
    if (mpv_get_property(m_mpv, "current-tracks/audio/id", MPV_FORMAT_INT64, &audioId) < 0)
        return;
    int64_t videoId = 0;
    const bool hasCover = mpv_get_property(m_mpv, "current-tracks/video/id", MPV_FORMAT_INT64, &videoId) >= 0;
    const int spectrumColumns = m_visualizerBands * 2;
    const QString spectrum = QStringLiteral(
        "[aid%1]asplit=3[ao][music][peakaudio];"
        "[music]showfreqs=s=%2x240:r=30:mode=bar:ascale=sqrt:fscale=log:win_size=2048:averaging=1:colors=0xd6ad55,"
        "drawgrid=w=2:h=ih:t=1:c=black,format=rgba,colorkey=black:0.12:0.18,scale=1280:300:flags=neighbor[spectrum];"
        "[peakaudio]showfreqs=s=%2x240:r=30:mode=dot:ascale=sqrt:fscale=log:win_size=2048:averaging=4:colors=0xf7dfa0,"
        "drawgrid=w=2:h=ih:t=1:c=black,format=rgba,colorkey=black:0.12:0.18,scale=1280:300:flags=neighbor[peaks];"
        "[spectrum][peaks]overlay=0:0:format=auto[bars];")
        .arg(audioId).arg(spectrumColumns);
    const QString background = hasCover
        ? QStringLiteral("[vid%1]scale=1280:720:force_original_aspect_ratio=decrease,pad=1280:720:(ow-iw)/2:(oh-ih)/2:black[background];").arg(videoId)
        : QStringLiteral("color=c=black:s=1280x720:r=30[background];");
    const QByteArray graph = (spectrum + background
        + QStringLiteral("[background][bars]overlay=0:H-h:format=auto[vo]")).toUtf8();
    mpv_set_property_string(m_mpv, "lavfi-complex", graph.constData());
}
void MpvItem::setPosition(double seconds) { if (m_mpv) mpv_set_property_async(m_mpv, 0, "time-pos", MPV_FORMAT_DOUBLE, &seconds); }
void MpvItem::setVolume(double value) { if (m_mpv) mpv_set_property_async(m_mpv, 0, "volume", MPV_FORMAT_DOUBLE, &value); }
void MpvItem::setFlag(const char *name, bool value) { int flag = value; mpv_set_property_async(m_mpv, 0, name, MPV_FORMAT_FLAG, &flag); }

void MpvItem::wakeup(void *context)
{
    QMetaObject::invokeMethod(static_cast<MpvItem *>(context), &MpvItem::processEvents, Qt::QueuedConnection);
}

void MpvItem::renderUpdate(void *context)
{
    auto *item = static_cast<MpvItem *>(context);
    QMetaObject::invokeMethod(item, [item] { item->update(); }, Qt::QueuedConnection);
}

void MpvItem::processEvents()
{
    if (!m_mpv) return;
    while (mpv_event *event = mpv_wait_event(m_mpv, 0)) {
        if (event->event_id == MPV_EVENT_NONE) break;
        if (event->event_id == MPV_EVENT_START_FILE) {
            if (!m_loading) {
                m_loading = true;
                emit loadingChanged();
            }
        } else if (event->event_id == MPV_EVENT_FILE_LOADED) {
            if (m_loading) {
                m_loading = false;
                emit loadingChanged();
            }
            configureMusicVisualizer();
        } else if (event->event_id == MPV_EVENT_END_FILE) {
            auto *end = static_cast<mpv_event_end_file *>(event->data);
            if (m_loading) {
                m_loading = false;
                emit loadingChanged();
            }
            if (end && end->reason == MPV_END_FILE_REASON_ERROR) {
                emit errorOccurred(QStringLiteral("Não foi possível abrir esta mídia. Verifique o link, a conexão e se o conteúdo está disponível na sua região.\n\nDetalhe: %1")
                                       .arg(QString::fromUtf8(mpv_error_string(end->error))));
            }
        } else if (event->event_id == MPV_EVENT_PROPERTY_CHANGE) {
            auto *p = static_cast<mpv_event_property *>(event->data);
            if (!p || !p->data) continue;
            const QByteArray name(p->name);
            if (name == "pause") { m_playing = !*static_cast<int *>(p->data); emit playingChanged(); }
            else if (name == "time-pos") { m_position = *static_cast<double *>(p->data); emit positionChanged(); }
            else if (name == "duration") { m_duration = *static_cast<double *>(p->data); emit durationChanged(); }
            else if (name == "volume") { m_volume = *static_cast<double *>(p->data); emit volumeChanged(); }
            else if (name == "media-title") { m_mediaTitle = QString::fromUtf8(*static_cast<char **>(p->data)); emit mediaTitleChanged(); }
        } else if (event->event_id == MPV_EVENT_LOG_MESSAGE) {
            continue;
        }
    }
}
