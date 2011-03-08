#include <QtDebug>
#include <QMutexLocker>

#include "effects/effectsbackend.h"

EffectsBackend::EffectsBackend(QObject* pParent, QString name)
        : QObject(pParent),
          m_name(name) {
}

EffectsBackend::~EffectsBackend() {
}

const QString EffectsBackend::getName() const {
    QMutexLocker locker(&m_mutex);
    return m_name;
}

void EffectsBackend::registerEffect(const QString id,
                                    EffectManifestPointer pManifest,
                                    EffectInstantiator pInstantiator) {
    QMutexLocker locker(&m_mutex);
    if (m_registeredEffects.contains(id)) {
        qDebug() << "WARNING: Effect" << id << "already registered";
        return;
    }

    m_registeredEffects[id] = QPair<EffectManifestPointer, EffectInstantiator>(pManifest, pInstantiator);
}

const QSet<QString> EffectsBackend::getEffectIds() const {
    QMutexLocker locker(&m_mutex);
    return QSet<QString>::fromList(m_registeredEffects.keys());
}

EffectManifestPointer EffectsBackend::getManifest(const QString effectId) const {
    QMutexLocker locker(&m_mutex);
    if (!m_registeredEffects.contains(effectId)) {
        qDebug() << "WARNING: Effect" << effectId << "is not registered.";
        return EffectManifestPointer();
    }
    return m_registeredEffects[effectId].first;
}

bool EffectsBackend::canInstantiateEffect(const QString effectId) const {
    QMutexLocker locker(&m_mutex);
    return m_registeredEffects.contains(effectId);
}

EffectPointer EffectsBackend::instantiateEffect(const QString effectId) {
    QMutexLocker locker(&m_mutex);
    if (!m_registeredEffects.contains(effectId)) {
        qDebug() << "WARNING: Effect" << effectId << "is not registered.";
        return EffectPointer();
    }
    QPair<EffectManifestPointer, EffectInstantiator> effectInfo = m_registeredEffects[effectId];

    return (*effectInfo.second)(this, effectInfo.first);
}

