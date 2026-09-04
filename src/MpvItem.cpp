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
#include <QtGlobal>

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
void MpvItem::setVideoBasic(double brightness, double contrast, double saturation,
                            double gamma, double hue, double sharpen,
                            bool deband, double grain)
{
    m_brightness = brightness;
    m_contrast = contrast;
    m_saturation = saturation;
    m_gamma = gamma;
    m_hue = hue;
    m_sharpen = sharpen;
    m_deband = deband;
    m_grain = grain;
    applyVideoFilters();
}

void MpvItem::setVideoCrop(int left, int right, int top, int bottom)
{
    m_cropLeft = qMax(0, left); m_cropRight = qMax(0, right);
    m_cropTop = qMax(0, top); m_cropBottom = qMax(0, bottom);
    applyVideoFilters();
}

void MpvItem::setVideoGeometry(int rotation, bool mirrorHorizontal,
                               bool mirrorVertical, double zoom)
{
    m_rotation = ((rotation % 360) + 360) % 360;
    m_mirrorHorizontal = mirrorHorizontal;
    m_mirrorVertical = mirrorVertical;
    m_zoom = qBound(0.5, zoom, 3.0);
    applyVideoFilters();
}

void MpvItem::setVideoColor(bool grayscale, bool negative, double sepia,
                            int posterizeLevels)
{
    m_grayscale = grayscale;
    m_negative = negative;
    m_sepia = qBound(0.0, sepia, 1.0);
    m_posterizeLevels = qBound(0, posterizeLevels, 64);
    applyVideoFilters();
}

void MpvItem::setVideoOther(bool deinterlace, double denoise, bool removeBanding)
{
    m_deinterlace = deinterlace;
    m_denoise = qBound(0.0, denoise, 10.0);
    m_removeBanding = removeBanding;
    applyVideoFilters();
}

void MpvItem::applyVideoFilters()
{
    if (!m_mpv) return;

    // Basic colour controls use mpv's OpenGL video equalizer. Unlike the old
    // implementation, values are sent continuously while a slider is moved.
    auto setEqualizer = [this](const char *name, double value) {
        int64_t rounded = qRound64(value);
        mpv_set_property(m_mpv, name, MPV_FORMAT_INT64, &rounded);
    };
    setEqualizer("brightness", m_brightness);
    setEqualizer("contrast", m_contrast);
    setEqualizer("saturation", m_saturation);
    setEqualizer("gamma", m_gamma);
    setEqualizer("hue", m_hue);

    int64_t rotate = m_rotation;
    mpv_set_property(m_mpv, "video-rotate", MPV_FORMAT_INT64, &rotate);
    double panscan = qBound(0.0, (m_zoom - 1.0) / 2.0, 1.0);
    mpv_set_property(m_mpv, "panscan", MPV_FORMAT_DOUBLE, &panscan);
    int deinterlace = m_deinterlace;
    mpv_set_property(m_mpv, "deinterlace", MPV_FORMAT_FLAG, &deinterlace);
    int deband = m_deband || m_removeBanding;
    mpv_set_property(m_mpv, "deband", MPV_FORMAT_FLAG, &deband);

    QStringList filters;
    if (m_cropLeft || m_cropRight || m_cropTop || m_cropBottom) {
        filters << QStringLiteral("lavfi=[crop=iw-%1:ih-%2:%3:%4]")
                       .arg(m_cropLeft + m_cropRight).arg(m_cropTop + m_cropBottom)
                       .arg(m_cropLeft).arg(m_cropTop);
    }
    if (m_mirrorHorizontal) filters << QStringLiteral("lavfi=[hflip]");
    if (m_mirrorVertical) filters << QStringLiteral("lavfi=[vflip]");
    if (m_sharpen > 0.01)
        filters << QStringLiteral("lavfi=[unsharp=5:5:%1:5:5:0]").arg(m_sharpen, 0, 'f', 2);
    if (m_grain > 0.01)
        filters << QStringLiteral("lavfi=[noise=alls=%1:allf=t]").arg(m_grain, 0, 'f', 1);
    if (m_denoise > 0.01)
        filters << QStringLiteral("lavfi=[hqdn3d=%1:%1:%2:%2]")
                       .arg(m_denoise, 0, 'f', 1).arg(m_denoise * 1.5, 0, 'f', 1);
    if (m_grayscale) filters << QStringLiteral("lavfi=[hue=s=0]");
    if (m_negative) filters << QStringLiteral("lavfi=[negate]");
    if (m_sepia > 0.01) {
        const double s = m_sepia;
        filters << QStringLiteral("lavfi=[colorchannelmixer=%1:%2:%3:0:%4:%5:%6:0:%7:%8:%9]")
                       .arg(1.0 - 0.607*s, 0, 'f', 3).arg(0.769*s, 0, 'f', 3).arg(0.189*s, 0, 'f', 3)
                       .arg(0.349*s, 0, 'f', 3).arg(1.0 - 0.314*s, 0, 'f', 3).arg(0.168*s, 0, 'f', 3)
                       .arg(0.272*s, 0, 'f', 3).arg(0.534*s, 0, 'f', 3).arg(1.0 - 0.869*s, 0, 'f', 3);
    }
    if (m_posterizeLevels >= 2)
        filters << QStringLiteral("lavfi=[lutrgb=r='floor(val/%1)*%1':g='floor(val/%1)*%1':b='floor(val/%1)*%1']")
                       .arg(qMax(1, 256 / m_posterizeLevels));

    const QByteArray vf = filters.join(',').toUtf8();
    mpv_set_property_string(m_mpv, "vf", vf.constData());
}

void MpvItem::resetVideoAdjustments()
{
    m_brightness = m_contrast = m_saturation = m_gamma = m_hue = 0.0;
    m_sharpen = m_grain = 0.0; m_deband = false;
    m_cropLeft = m_cropRight = m_cropTop = m_cropBottom = 0;
    m_rotation = 0; m_mirrorHorizontal = m_mirrorVertical = false; m_zoom = 1.0;
    m_grayscale = m_negative = false; m_sepia = 0.0; m_posterizeLevels = 0;
    m_deinterlace = false; m_denoise = 0.0; m_removeBanding = false;
    applyVideoFilters();
}
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
