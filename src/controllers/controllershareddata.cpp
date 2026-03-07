#include "controllers/controllershareddata.h"

#include "moc_controllershareddata.cpp"

ControllerSharedData::ControllerSharedData(QObject* parent)
        : QObject(parent) {
}

QVariant ControllerSharedData::get(const QString& ns,
        const QString& entity,
        const QString& key) const {
    auto nsIt = m_values.constFind(ns);
    if (nsIt == m_values.constEnd()) {
        return QVariant();
    }
    auto entityIt = nsIt->constFind(entity);
    if (entityIt == nsIt->constEnd()) {
        return QVariant();
    }
    auto keyIt = entityIt->constFind(key);
    if (keyIt == entityIt->constEnd()) {
        return QVariant();
    }
    return *keyIt;
}

void ControllerSharedData::set(const QString& ns,
        const QString& entity,
        const QString& key,
        const QVariant& value,
        QObject* sender) {
    // QHash default-constructs entries if they do not exist, no need to check if they exist
    // already.
    m_values[ns][entity][key] = value;
    emit updated(ns, entity, key, value, sender);
}

ControllerNamespacedSharedData* ControllerSharedData::namespaced(
        const QString& ns) {
    return new ControllerNamespacedSharedData(this, ns);
}

ControllerNamespacedSharedData::ControllerNamespacedSharedData(
        ControllerSharedData* parent, const QString& ns)
        : QObject(parent),
          m_namespace(ns) {
    connect(parent,
            &ControllerSharedData::updated,
            this,
            [this](const QString& ns,
                    const QString& entity,
                    const QString& key,
                    const QVariant& value,
                    QObject* sender) {
                if (ns != m_namespace) {
                    return;
                }
                emit updated(entity, key, value, sender);
            });
}

QVariant ControllerNamespacedSharedData::get(
        const QString& entity, const QString& key) const {
    return static_cast<ControllerSharedData*>(parent())->get(
            m_namespace, entity, key);
}

void ControllerNamespacedSharedData::set(const QString& entity,
        const QString& key,
        const QVariant& value,
        QObject* sender) {
    static_cast<ControllerSharedData*>(parent())->set(
            m_namespace, entity, key, value, sender);
}
