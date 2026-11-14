#include "controllers/controllershareddata.h"

#include <QReadLocker>
#include <QWriteLocker>

#include "moc_controllershareddata.cpp"

ControllerSharedData::ControllerSharedData(QObject* parent)
        : QObject(parent) {
}

QVariant ControllerSharedData::get(const QString& ns,
        const QString& entity,
        const QString& key) const {
    QReadLocker locker(&m_lock);
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

QHash<QString, QHash<QString, QVariant>> ControllerSharedData::getAll(
        const QString& ns) const {
    QReadLocker locker(&m_lock);
    auto nsIt = m_values.constFind(ns);
    if (nsIt == m_values.constEnd()) {
        return {};
    }
    return *nsIt;
}

void ControllerSharedData::set(const QString& ns,
        const QString& entity,
        const QString& key,
        const QVariant& value,
        QObject* sender) {
    {
        QWriteLocker locker(&m_lock);
        // QHash default-constructs entries if they do not exist, no need to
        // check if they exist already.
        const auto existing = m_values[ns][entity].find(key);
        if (existing != m_values[ns][entity].cend()) {
            if (existing->metaType() != value.metaType()) {
                qWarning() << "Different type when updating shared data value" << ns << entity << key
                << "was" << existing->typeName() << ", new type is " << value.typeName() << ". Updating anyway.";
            }
        }
        m_values[ns][entity][key] = value;
    }
    // Emit outside the lock to avoid holding it during signal delivery,
    // which could deadlock if a slot calls back into get().
    emit updated(ns, entity, key, value, sender);
}

std::shared_ptr<ControllerNamespacedSharedData> ControllerSharedData::namespaced(
        const QString& ns) {
    return std::make_shared<ControllerNamespacedSharedData>(this, ns);
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

QHash<QString, QHash<QString, QVariant>> ControllerNamespacedSharedData::getAll() const {
    return static_cast<ControllerSharedData*>(parent())->getAll(m_namespace);
}

void ControllerNamespacedSharedData::set(const QString& entity,
        const QString& key,
        const QVariant& value,
        QObject* sender) {
    static_cast<ControllerSharedData*>(parent())->set(
            m_namespace, entity, key, value, sender);
}
