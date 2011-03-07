#include <QtDebug>

#include "effects/effectsbackend.h"

EffectsBackend::EffectsBackend(QObject* pParent, QString name)
        : QObject(pParent),
          m_name(name) {
}

EffectsBackend::~EffectsBackend() {
}

const QString EffectsBackend::getName() const {
    return m_name;
}

void EffectsBackend::registerEffect(const QString id,
                                    EffectManifestPointer pManifest,
                                    EffectInstantiator pInstantiator) {
    if (m_registeredEffects.contains(id)) {
        qDebug() << "WARNING: Effect" << id << "already registered";
        return;
    }

    m_registeredEffects[id] = QPair<EffectManifestPointer, EffectInstantiator>(pManifest, pInstantiator);
}

const QList<QString> EffectsBackend::getEffectIds() const {
    return m_registeredEffects.keys();
}

EffectManifestPointer EffectsBackend::getManifest(const QString effectId) const {
    if (!m_registeredEffects.contains(effectId)) {
        qDebug() << "WARNING: Effect" << effectId << "is not registered.";
        return EffectManifestPointer();
    }
    return m_registeredEffects[effectId].first;
}

EffectPointer EffectsBackend::instantiateEffect(const QString effectId) {
    if (!m_registeredEffects.contains(effectId)) {
        qDebug() << "WARNING: Effect" << effectId << "is not registered.";
        return EffectPointer();
    }
    QPair<EffectManifestPointer, EffectInstantiator> effectInfo = m_registeredEffects[effectId];

    return (*effectInfo.second)(this, effectInfo.first);
}

