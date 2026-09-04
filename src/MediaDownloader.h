// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Alessandro Henriques Teixeira — Studio Arn

#pragma once

#include <QObject>
#include <QProcess>
#include <QUrl>

class MediaDownloader final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)

public:
    explicit MediaDownloader(QObject *parent = nullptr);
    bool running() const { return m_running; }
    int progress() const { return m_progress; }
    QString status() const { return m_status; }

    Q_INVOKABLE void download(const QString &url, const QUrl &folder, bool audioOnly);
    Q_INVOKABLE void cancel();

signals:
    void runningChanged();
    void progressChanged();
    void statusChanged();
    void finished(const QString &message);
    void errorOccurred(const QString &message);

private:
    void setStatus(const QString &status);
    QProcess m_process;
    bool m_running = false;
    int m_progress = 0;
    QString m_status;
};
