#include "mixxxcontroller.h"

namespace mixxx {
namespace qml {

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
        const auto* const meta = childComponent->metaObject();
        const auto idx = meta->indexOfMethod(kInitSignature);
        if (idx >= 0) {
            const auto method = meta->method(idx);
            if (method.isValid()) {
                method.invoke(childComponent, Qt::DirectConnection);
            } else {
                // TODO: log?
            }
        }
    }
}

void MixxxController::shutdownChildrenComponents() {
    for (auto* childComponent : m_children) {
        const auto* const meta = childComponent->metaObject();
        const auto idx = meta->indexOfMethod(kShutdownSignature);
        if (idx >= 0) {
            const auto method = meta->method(idx);
            if (method.isValid()) {
                method.invoke(childComponent, Qt::DirectConnection);
            } else {
                // TODO: log?
            }
        }
    }
}

} // namespace qml
} // namespace mixxx
