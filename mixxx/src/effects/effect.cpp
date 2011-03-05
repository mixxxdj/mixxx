#include <QtDebug>

#include "effects/effect.h"
#include "effects/effectsbackend.h"

Effect::Effect(EffectsBackend* pBackend, EffectManifest& effectManifest)
        : QObject(pBackend),
          m_pEffectsBackend(pBackend),
          m_effectManifest(effectManifest) {
    foreach (const EffectManifestParameter& parameter, m_effectManifest.parameters()) {
        EffectParameter* pParameter = new EffectParameter(this, parameter);
        m_parameters.append(EffectParameterPointer(pParameter, &QObject::deleteLater));
    }
}

Effect::~Effect() {
}

const EffectManifest& Effect::getManifest() const {
    return m_effectManifest;
}

unsigned int Effect::numParameters() const {
    return m_parameters.size();
}

EffectParameterPointer Effect::getParameter(unsigned int parameterNumber) {
    if (parameterNumber >= m_parameters.size()) {
        qDebug() << "WARNING: Invalid parameter index.";
        return EffectParameterPointer();
    }
    return m_parameters[parameterNumber];
}
