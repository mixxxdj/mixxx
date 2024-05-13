#include <controllers/controllershareddata.h>

#include "moc_controllershareddata.cpp"

ControllerNamespacedSharedData* ControllerSharedData::namespaced(const QString& ns) {
    return new ControllerNamespacedSharedData(this, ns);
}

ControllerNamespacedSharedData::ControllerNamespacedSharedData(
        ControllerSharedData* parent, const QString& ns)
        : QObject(parent), m_namespace(ns) {
    connect(parent,
            &ControllerSharedData::updated,
            this,
            [this](const QString& ns, const QVariant& value) {
                if (ns != m_namespace) {
                    return;
                }
                emit updated(value);
            });
}
