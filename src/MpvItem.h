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
    Q_INVOKABLE void setVideoBasic(double brightness, double contrast, double saturation,
                                   double gamma, double hue, double sharpen,
                                   bool deband, double grain);
    Q_INVOKABLE void setVideoCrop(int left, int right, int top, int bottom);
    Q_INVOKABLE void setVideoGeometry(int rotation, bool mirrorHorizontal,
                                      bool mirrorVertical, double zoom);
    Q_INVOKABLE void setVideoColor(bool grayscale, bool negative, double sepia,
                                   int posterizeLevels);
    Q_INVOKABLE void setVideoOther(bool deinterlace, double denoise,
                                   bool removeBanding);
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
    void applyVideoFilters();

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
    double m_brightness = 0.0;
    double m_contrast = 0.0;
    double m_saturation = 0.0;
    double m_gamma = 0.0;
    double m_hue = 0.0;
    double m_sharpen = 0.0;
    bool m_deband = false;
    double m_grain = 0.0;
    int m_cropLeft = 0;
    int m_cropRight = 0;
    int m_cropTop = 0;
    int m_cropBottom = 0;
    int m_rotation = 0;
    bool m_mirrorHorizontal = false;
    bool m_mirrorVertical = false;
    double m_zoom = 1.0;
    bool m_grayscale = false;
    bool m_negative = false;
    double m_sepia = 0.0;
    int m_posterizeLevels = 0;
    bool m_deinterlace = false;
    double m_denoise = 0.0;
    bool m_removeBanding = false;
};
