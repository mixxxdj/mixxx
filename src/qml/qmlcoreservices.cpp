#include "qml/qmlcoreservices.h"

#include "moc_qmlcoreservices.cpp"

namespace mixxx {
namespace qml {

// QmlCoreServices::QmlCoreServices(QObject* parent, int): QObject(parent){

// }

void QmlCoreServices::setReady() {
    if (m_ready) {
        return;
    }
    m_ready = true;
    emit readyChanged();
}

// static
QmlCoreServices* QmlCoreServices::create(QQmlEngine* pQmlEngine, QJSEngine*) {
    return instance();
}

} // namespace qml
} // namespace mixxx
