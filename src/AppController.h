#pragma once

#include <QGuiApplication>
#include <QUrl>
#include <QVariantList>

class AppController final : public QGuiApplication
{
    Q_OBJECT

public:
    AppController(int &argc, char **argv);
    Q_INVOKABLE QVariantList takePendingMedia();
    void enqueueMedia(const QUrl &url);

signals:
    void mediaOpenRequested(const QUrl &url);

protected:
    bool event(QEvent *event) override;

private:
    QVariantList m_pendingMedia;
    bool m_qmlReady = false;
};
