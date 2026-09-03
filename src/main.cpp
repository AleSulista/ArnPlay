// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Alessandro Henriques Teixeira — Studio Arn

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QtQml>

#include "MpvItem.h"

int main(int argc, char *argv[])
{
    QGuiApplication::setApplicationName(QStringLiteral("ArnPlay"));
    QGuiApplication::setOrganizationName(QStringLiteral("Studio Arn"));
    QGuiApplication::setOrganizationDomain(QStringLiteral("studioarn.com.br"));

    // libmpv renders through OpenGL. Qt Quick must use the same graphics API.
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

    QGuiApplication app(argc, argv);
    qmlRegisterType<MpvItem>("StudioArn.ArnPlay", 1, 0, "MpvPlayer");

    QQmlApplicationEngine engine;
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, [] { QCoreApplication::exit(EXIT_FAILURE); },
                     Qt::QueuedConnection);
    engine.loadFromModule("StudioArn.ArnPlay", "Main");
    return app.exec();
}
