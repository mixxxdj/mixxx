#include <QtDebug>

#include "effects/effectsbackend.h"
#include "effects/effectsmanager.h"

EffectsBackend::EffectsBackend(QObject* pParent, QString name)
        : QObject(pParent),
          m_name(name) {
}

EffectsBackend::~EffectsBackend() {
    m_registeredEffects.clear();
    m_effectIds.clear();
}

const QString EffectsBackend::getName() const {
    return m_name;
}

void EffectsBackend::registerEffect(const QString& id,
                                    EffectManifestPointer pManifest,
                                    EffectInstantiatorPointer pInstantiator) {
    if (m_registeredEffects.contains(id)) {
        qWarning() << "WARNING: Effect" << id << "already registered";
        return;
    }

    m_registeredEffects[id] = RegisteredEffect(pManifest, pInstantiator);
    m_effectIds.append(id);
    emit(effectRegistered(pManifest));
}

const QList<QString> EffectsBackend::getEffectIds() const {
    return m_effectIds;
}

EffectManifestPointer EffectsBackend::getManifest(const QString& effectId) const {
    if (!m_registeredEffects.contains(effectId)) {
        qWarning() << "WARNING: Effect" << effectId << "is not registered.";
        return EffectManifestPointer();
    }
    return m_registeredEffects[effectId].manifest();
}

bool EffectsBackend::canInstantiateEffect(const QString& effectId) const {
    return m_registeredEffects.contains(effectId);
}

EffectPointer EffectsBackend::instantiateEffect(EffectsManager* pEffectsManager,
                                                const QString& effectId) {
    if (!m_registeredEffects.contains(effectId)) {
        qWarning() << "WARNING: Effect" << effectId << "is not registered.";
        return EffectPointer();
    }
    RegisteredEffect& effectInfo = m_registeredEffects[effectId];

    return EffectPointer(new Effect(pEffectsManager,
                                    effectInfo.manifest(),
                                    effectInfo.initiator()));
}
