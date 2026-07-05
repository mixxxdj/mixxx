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

void QmlCoreServices::addOpenedPopup(QObject* popup) {
    bool wasEmpty = m_popups.isEmpty();
    m_popups.insert(popup);
    if (wasEmpty) {
        emit hasPopupChanged();
    }
}
void QmlCoreServices::removeOpenedPopup(QObject* popup) {
    bool wasNotEmpty = !m_popups.isEmpty();
    m_popups.remove(popup);
    if (wasNotEmpty) {
        emit hasPopupChanged();
    }
}

void QmlCoreServices::clearOpenedPopup() {
    if (m_popups.isEmpty()) {
        return;
    }
    m_popups.clear();
    emit hasPopupChanged();
}
} // namespace qml
} // namespace mixxx
