#include "qml/qmlmixxxcontrollerscreen.h"

#include <QSGNode>
#include <QSGSimpleRectNode>

#include "moc_qmlmixxxcontrollerscreen.cpp"

namespace mixxx {
namespace qml {

QmlMixxxControllerScreen::QmlMixxxControllerScreen(QQuickItem* parent)
        : QQuickItem(parent) {
}

void QmlMixxxControllerScreen::setTransform(const QJSValue& value) {
    if (!value.isCallable()) {
        return;
    }
    m_transformFunc = value;
    emit transformChanged();
}

void QmlMixxxControllerScreen::setInit(const QJSValue& value) {
    if (!value.isCallable()) {
        return;
    }
    m_initFunc = value;
    emit initChanged();
}

void QmlMixxxControllerScreen::setShutdown(const QJSValue& value) {
    if (!value.isCallable()) {
        return;
    }
    m_shutdownFunc = value;
    emit shutdownChanged();
}

} // namespace qml
} // namespace mixxx
