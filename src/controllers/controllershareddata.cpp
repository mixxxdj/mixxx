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
        m_values[ns][entity][key] = value;
    }
    // Emit outside the lock to avoid holding it during signal delivery,
    // which could deadlock if a slot calls back into get().
    emit updated(ns, entity, key, value, sender);
}

void ControllerSharedData::setMultiple(const QString& ns,
        const QHash<QString, QHash<QString, QVariant>>& values,
        QObject* sender) {
    {
        QWriteLocker locker(&m_lock);
        auto& nsMap = m_values[ns];
        for (auto entityIt = values.constBegin();
                entityIt != values.constEnd();
                ++entityIt) {
            auto& entityMap = nsMap[entityIt.key()];
            for (auto keyIt = entityIt->constBegin();
                    keyIt != entityIt->constEnd();
                    ++keyIt) {
                entityMap[keyIt.key()] = keyIt.value();
            }
        }
    }
    // Emit individual signals outside the lock.
    for (auto entityIt = values.constBegin();
            entityIt != values.constEnd();
            ++entityIt) {
        for (auto keyIt = entityIt->constBegin();
                keyIt != entityIt->constEnd();
                ++keyIt) {
            emit updated(ns, entityIt.key(), keyIt.key(), keyIt.value(), sender);
        }
    }
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

void ControllerNamespacedSharedData::setMultiple(
        const QHash<QString, QHash<QString, QVariant>>& values,
        QObject* sender) {
    static_cast<ControllerSharedData*>(parent())->setMultiple(
            m_namespace, values, sender);
}
