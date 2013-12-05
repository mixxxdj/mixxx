#include <QtDebug>

#include "effects/effect.h"
#include "effects/effectprocessor.h"
#include "effects/effectsmanager.h"
#include "engine/effects/engineeffect.h"

Effect::Effect(QObject* pParent, EffectsManager* pEffectsManager,
               const EffectManifest& manifest,
               EffectInstantiatorPointer pInstantiator)
        : QObject(pParent),
          m_pEffectsManager(pEffectsManager),
          m_manifest(manifest),
          m_pEngineEffect(new EngineEffect(manifest, pInstantiator)) {
    foreach (const EffectManifestParameter& parameter, m_manifest.parameters()) {
        EffectParameter* pParameter = new EffectParameter(
            this, pEffectsManager, m_parameters.size(), parameter);
        m_parameters.append(pParameter);
        if (m_parametersById.contains(parameter.id())) {
            qDebug() << debugString() << "WARNING: Loaded EffectManifest that had parameters with duplicate IDs. Dropping one of them.";
        }
        m_parametersById[parameter.id()] = pParameter;
    }
}

Effect::~Effect() {
    qDebug() << debugString() << "destroyed";
    m_parametersById.clear();
    for (int i = 0; i < m_parameters.size(); ++i) {
        EffectParameter* pParameter = m_parameters.at(i);
        m_parameters[i] = NULL;
        delete pParameter;
    }
}

void Effect::updateEngineState() {
    foreach (EffectParameter* pParameter, m_parameters) {
        pParameter->updateEngineState();
    }
}

EngineEffect* Effect::getEngineEffect() {
    return m_pEngineEffect;
}

const EffectManifest& Effect::getManifest() const {
    return m_manifest;
}

unsigned int Effect::numParameters() const {
    return m_parameters.size();
}

EffectParameter* Effect::getParameterById(const QString& id) const {
    EffectParameter* pParameter = m_parametersById.value(id, NULL);
    if (pParameter == NULL) {
        qDebug() << debugString() << "getParameterById"
                 << "WARNING: parameter for id does not exist:" << id;
    }
    return pParameter;
}

EffectParameter* Effect::getParameter(unsigned int parameterNumber) {
    EffectParameter* pParameter = m_parameters.value(parameterNumber, NULL);
    if (pParameter == NULL) {
        qDebug() << debugString() << "WARNING: Invalid parameter index.";
    }
    return pParameter;
}
