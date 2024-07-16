#include "mixxxcontroller.h"

namespace mixxx {
namespace qml {

MixxxController::MixxxController(QObject* parent)
        : QObject(parent), m_pChildren(this, &m_children) {
}

void MixxxController::classBegin() {
}

void MixxxController::componentComplete() {
    QObject::connect(this,
            &MixxxController::init,
            this,
            &MixxxController::initChildrenComponents);
    QObject::connect(this,
            &MixxxController::shutdown,
            this,
            &MixxxController::shutdownChildrenComponents);
}

void MixxxController::initChildrenComponents() {
    for (auto* childComponent : m_children) {
        // Try emit init signal
        QMetaObject::invokeMethod(childComponent, "init", Qt::DirectConnection);
    }
}

void MixxxController::shutdownChildrenComponents() {
    for (auto* childComponent : m_children) {
        // Try emit shutdown signal
        QMetaObject::invokeMethod(childComponent, "shutdown", Qt::DirectConnection);
    }
}

} // namespace qml
} // namespace mixxx
