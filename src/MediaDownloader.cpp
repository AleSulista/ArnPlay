// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Alessandro Henriques Teixeira — Studio Arn

#include "MediaDownloader.h"

#include <QDir>
#include <QRegularExpression>
#include <QStandardPaths>

MediaDownloader::MediaDownloader(QObject *parent) : QObject(parent)
{
    connect(&m_process, &QProcess::readyReadStandardOutput, this, [this] {
        const QString output = QString::fromUtf8(m_process.readAllStandardOutput());
        static const QRegularExpression percentExpression(QStringLiteral(R"(\[download\]\s+([0-9]+(?:\.[0-9]+)?)%)"));
        const auto match = percentExpression.match(output);
        if (match.hasMatch()) {
            const int value = qBound(0, qRound(match.captured(1).toDouble()), 100);
            if (m_progress != value) {
                m_progress = value;
                emit progressChanged();
            }
            setStatus(QStringLiteral("Baixando… %1%").arg(value));
        }
    });
    connect(&m_process, &QProcess::readyReadStandardError, this, [this] {
        const QString output = QString::fromUtf8(m_process.readAllStandardError()).trimmed();
        if (!output.isEmpty()) setStatus(output.section('\n', -1));
    });
    connect(&m_process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
            [this](int exitCode, QProcess::ExitStatus exitStatus) {
        m_running = false;
        emit runningChanged();
        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
            m_progress = 100;
            emit progressChanged();
            setStatus(QStringLiteral("Download concluído"));
            emit finished(QStringLiteral("Download concluído com sucesso."));
        } else {
            setStatus(QStringLiteral("Falha no download"));
            emit errorOccurred(QStringLiteral("Não foi possível concluir o download. Verifique o link, a conexão, a disponibilidade regional e sua autorização de acesso."));
        }
    });
    connect(&m_process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError) {
        if (!m_running) return;
        m_running = false;
        emit runningChanged();
        setStatus(QStringLiteral("Falha ao iniciar o download"));
        emit errorOccurred(QStringLiteral("Não foi possível iniciar o yt-dlp."));
    });
}

void MediaDownloader::download(const QString &url, const QUrl &folder, bool audioOnly)
{
    if (m_running) return;
    const QUrl mediaUrl(url.trimmed());
    if (!mediaUrl.isValid() || (mediaUrl.scheme() != "https" && mediaUrl.scheme() != "http")) {
        emit errorOccurred(QStringLiteral("Informe um endereço válido iniciado por https://"));
        return;
    }
    if (!folder.isLocalFile() || !QDir(folder.toLocalFile()).exists()) {
        emit errorOccurred(QStringLiteral("Escolha uma pasta válida para salvar."));
        return;
    }
    const QString program = QStandardPaths::findExecutable(QStringLiteral("yt-dlp"));
    if (program.isEmpty()) {
        emit errorOccurred(QStringLiteral("O yt-dlp não foi encontrado. Instale com: brew install yt-dlp"));
        return;
    }
    QStringList arguments{QStringLiteral("--no-playlist"), QStringLiteral("--newline"),
                          QStringLiteral("--progress"), QStringLiteral("--restrict-filenames")};
    if (audioOnly)
        arguments << "--extract-audio" << "--audio-format" << "mp3";
    else
        arguments << "--format" << "bv*+ba/b" << "--merge-output-format" << "mp4";
    arguments << "--output" << QDir(folder.toLocalFile()).filePath(QStringLiteral("%(title)s.%(ext)s"));
    arguments << mediaUrl.toString();
    m_progress = 0;
    emit progressChanged();
    m_running = true;
    emit runningChanged();
    setStatus(QStringLiteral("Preparando download…"));
    m_process.start(program, arguments);
}

void MediaDownloader::cancel()
{
    if (m_running) m_process.terminate();
}

void MediaDownloader::setStatus(const QString &status)
{
    if (m_status == status) return;
    m_status = status;
    emit statusChanged();
}
