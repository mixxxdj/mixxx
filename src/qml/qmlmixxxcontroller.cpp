#include "qml/qmlmixxxcontroller.h"

#include <QSGNode>
#include <QSGSimpleRectNode>

#include "moc_qmlmixxxcontroller.cpp"

namespace mixxx {
namespace qml {

QmlMixxxController::QmlMixxxController(QQuickItem* parent)
        : QQuickItem(parent) {
}

void QmlMixxxController::setTransform(const QJSValue& value) {
    if (!value.isCallable()) {
        return;
    }
    m_transformFunc = value;
    emit transformChanged();
}

void QmlMixxxController::setInit(const QJSValue& value) {
    if (!value.isCallable()) {
        return;
    }
    m_initFunc = value;
    emit initChanged();
}

void QmlMixxxController::setShutdown(const QJSValue& value) {
    if (!value.isCallable()) {
        return;
    }
    m_shutdownFunc = value;
    emit shutdownChanged();
}

} // namespace qml
} // namespace mixxx
