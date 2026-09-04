// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Alessandro Henriques Teixeira — Studio Arn

#pragma once

#include <QMutex>
#include <QQuickFramebufferObject>
#include <QUrl>

struct mpv_handle;
struct mpv_render_context;

class MpvItem : public QQuickFramebufferObject
{
    Q_OBJECT
    Q_PROPERTY(bool playing READ playing NOTIFY playingChanged)
    Q_PROPERTY(double position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(double duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(double volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(QString mediaTitle READ mediaTitle NOTIFY mediaTitleChanged)
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)

public:
    explicit MpvItem(QQuickItem *parent = nullptr);
    ~MpvItem() override;

    Renderer *createRenderer() const override;

    bool playing() const { return m_playing; }
    double position() const { return m_position; }
    double duration() const { return m_duration; }
    double volume() const { return m_volume; }
    QString mediaTitle() const { return m_mediaTitle; }
    bool loading() const { return m_loading; }

    Q_INVOKABLE void open(const QUrl &url);
    Q_INVOKABLE void togglePause();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void seek(double seconds);
    Q_INVOKABLE void addSubtitle(const QUrl &url);
    Q_INVOKABLE void cycleAudioTrack();
    Q_INVOKABLE void cycleSubtitleTrack();
    Q_INVOKABLE void setPlaybackSpeed(double speed);
    Q_INVOKABLE void setVideoAdjustments(int brightness, int contrast, int saturation, int gamma, int rotation);
    Q_INVOKABLE void resetVideoAdjustments();
    Q_INVOKABLE void setAudioAdjustments(int bass, int treble, bool normalize, double delay);
    Q_INVOKABLE void resetAudioAdjustments();
    Q_INVOKABLE void setMusicVisualizer(bool enabled, int bands = 64);
    void setPosition(double seconds);
    void setVolume(double value);

signals:
    void playingChanged();
    void positionChanged();
    void durationChanged();
    void volumeChanged();
    void mediaTitleChanged();
    void loadingChanged();
    void errorOccurred(const QString &message);

private slots:
    void processEvents();

private:
    friend class MpvRenderer;
    static void wakeup(void *context);
    static void renderUpdate(void *context);
    void setFlag(const char *name, bool value);
    void configureMusicVisualizer();

    mpv_handle *m_mpv = nullptr;
    mpv_render_context *m_renderContext = nullptr;
    mutable QMutex m_renderMutex;
    bool m_playing = false;
    double m_position = 0.0;
    double m_duration = 0.0;
    double m_volume = 80.0;
    QString m_mediaTitle;
    bool m_loading = false;
    bool m_musicVisualizer = true;
    bool m_currentMediaIsAudio = false;
    int m_visualizerBands = 64;
};
