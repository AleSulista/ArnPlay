// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Alessandro Henriques Teixeira — Studio Arn

#include <QGuiApplication>
#include <QFileInfo>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QtQml>

#include "MpvItem.h"
#include "MediaDownloader.h"
#include "AppController.h"

int main(int argc, char *argv[])
{
    // Applications started from Finder do not inherit Homebrew's executable
    // paths. Make yt-dlp available to libmpv and to the download controller.
    const QByteArray inheritedPath = qgetenv("PATH");
    qputenv("PATH", QByteArrayLiteral("/usr/local/bin:/opt/homebrew/bin:") + inheritedPath);

    QGuiApplication::setApplicationName(QStringLiteral("ArnPlay"));
    QGuiApplication::setOrganizationName(QStringLiteral("Studio Arn"));
    QGuiApplication::setOrganizationDomain(QStringLiteral("studioarn.com.br"));

    // libmpv renders through OpenGL. Qt Quick must use the same graphics API.
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

    AppController app(argc, argv);
    AppController &appController = app;

    for (int argumentIndex = 1; argumentIndex < argc; ++argumentIndex) {
        const QFileInfo mediaFile(QString::fromLocal8Bit(argv[argumentIndex]));
        if (mediaFile.exists() && mediaFile.isFile())
            appController.enqueueMedia(QUrl::fromLocalFile(mediaFile.absoluteFilePath()));
    }

    qmlRegisterType<MpvItem>("StudioArn.ArnPlay", 1, 0, "MpvPlayer");

    QQmlApplicationEngine engine;
    MediaDownloader mediaDownloader;
    engine.rootContext()->setContextProperty("mediaDownloader", &mediaDownloader);
    engine.rootContext()->setContextProperty("appController", &appController);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, [] { QCoreApplication::exit(EXIT_FAILURE); },
                     Qt::QueuedConnection);
    engine.loadFromModule("StudioArn.ArnPlay", "Main");
    return app.exec();
}
