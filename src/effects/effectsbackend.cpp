#include <QtDebug>

#include "effects/effectsbackend.h"
#include "effects/effectsmanager.h"

EffectsBackend::EffectsBackend(QObject* pParent, QString name)
        : QObject(pParent),
          m_name(name) {
}

EffectsBackend::~EffectsBackend() {
    m_registeredEffects.clear();
}

const QString EffectsBackend::getName() const {
    return m_name;
}

void EffectsBackend::registerEffect(const QString& id,
                                    const EffectManifest& manifest,
                                    EffectInstantiatorPointer pInstantiator) {
    if (m_registeredEffects.contains(id)) {
        qWarning() << "WARNING: Effect" << id << "already registered";
        return;
    }

    m_registeredEffects[id] = QPair<EffectManifest, EffectInstantiatorPointer>(
        manifest, pInstantiator);
    emit(effectRegistered());
}

const QSet<QString> EffectsBackend::getEffectIds() const {
    return QSet<QString>::fromList(m_registeredEffects.keys());
}

EffectManifest EffectsBackend::getManifest(const QString& effectId) const {
    if (!m_registeredEffects.contains(effectId)) {
        qWarning() << "WARNING: Effect" << effectId << "is not registered.";
        return EffectManifest();
    }
    return m_registeredEffects[effectId].first;
}

bool EffectsBackend::canInstantiateEffect(const QString& effectId) const {
    return m_registeredEffects.contains(effectId);
}

Effect* EffectsBackend::instantiateEffect(EffectsManager* pEffectsManager,
                                                const QString& effectId) {
    if (!m_registeredEffects.contains(effectId)) {
        qWarning() << "WARNING: Effect" << effectId << "is not registered.";
        return NULL;
    }
    QPair<EffectManifest, EffectInstantiatorPointer>& effectInfo =
            m_registeredEffects[effectId];

    return new Effect(pEffectsManager, effectInfo.first, effectInfo.second);
}
