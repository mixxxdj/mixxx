#include "qml/qmlcoreservices.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QtGlobal>

#include "moc_qmlcoreservices.cpp"

namespace mixxx {
namespace qml {

void QmlCoreServices::setReady() {
    if (m_ready) {
        return;
    }
    m_ready = true;
    emit readyChanged();
}

void QmlCoreServices::setInitializationProgress(int progress, const QString& service) {
    const int clampedProgress = qBound(0, progress, 100);
    if (m_initializationProgress != clampedProgress) {
        m_initializationProgress = clampedProgress;
        emit initializationProgressChanged();
    }
    if (m_initializationService != service) {
        m_initializationService = service;
        emit initializationServiceChanged();
    }

    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
}

// static
QmlCoreServices* QmlCoreServices::create(QQmlEngine*, QJSEngine*) {
    return instance();
}

} // namespace qml
} // namespace mixxx
