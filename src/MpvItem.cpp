// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Alessandro Henriques Teixeira — Studio Arn

#include "MpvItem.h"

#include <QMetaObject>
#include <QDebug>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFramebufferObjectFormat>
#include <QQuickWindow>

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
    const QByteArray path = url.isLocalFile() ? url.toLocalFile().toUtf8() : url.toString().toUtf8();
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
        if (event->event_id == MPV_EVENT_PROPERTY_CHANGE) {
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
