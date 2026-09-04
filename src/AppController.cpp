#include "AppController.h"

#include <QEvent>
#include <QFileOpenEvent>

AppController::AppController(int &argc, char **argv)
    : QGuiApplication(argc, argv)
{
}

void AppController::enqueueMedia(const QUrl &url)
{
    if (!url.isValid() || url.isEmpty())
        return;

    if (m_qmlReady)
        emit mediaOpenRequested(url);
    else
        m_pendingMedia.append(QVariant::fromValue(url));
}

QVariantList AppController::takePendingMedia()
{
    m_qmlReady = true;
    const QVariantList pending = m_pendingMedia;
    m_pendingMedia.clear();
    return pending;
}

bool AppController::event(QEvent *event)
{
    if (event->type() == QEvent::FileOpen) {
        auto *openEvent = static_cast<QFileOpenEvent *>(event);
        enqueueMedia(openEvent->url().isEmpty()
                         ? QUrl::fromLocalFile(openEvent->file())
                         : openEvent->url());
        return true;
    }
    return QGuiApplication::event(event);
}
