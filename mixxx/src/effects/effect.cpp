#include <QtDebug>

#include "effects/effect.h"
#include "effects/effectsbackend.h"

Effect::Effect(EffectsBackend* pBackend, const EffectManifest& effectManifest)
        : QObject(pBackend),
          m_pEffectsBackend(pBackend),
          m_effectManifest(effectManifest) {
    foreach (const EffectManifestParameter& parameter, m_effectManifest.parameters()) {
        EffectParameterPointer pParameter(new EffectParameter(this, parameter),
                                          &QObject::deleteLater);
        m_parameters.append(pParameter);
        if (m_parametersById.contains(parameter.id())) {
            qDebug() << debugString() << "WARNING: Loaded EffectManifest that had parameters with duplicate IDs. Dropping one of them.";
        }
        m_parametersById[parameter.id()] = pParameter;
    }
}

Effect::~Effect() {
    m_parameters.clear();
    m_parametersById.clear();
}

const EffectManifest& Effect::getManifest() const {
    return m_effectManifest;
}

unsigned int Effect::numParameters() const {
    return m_parameters.size();
}

EffectParameterPointer Effect::getParameterFromId(const QString id) const {
    if (m_parametersById.contains(id)) {
        return m_parametersById[id];
    }
    qDebug() << debugString() << "parameterFromId" << "WARNING: parameter for id does not exist:" << id;
    return EffectParameterPointer();
}

EffectParameterPointer Effect::getParameter(unsigned int parameterNumber) {
    if (parameterNumber >= m_parameters.size()) {
        qDebug() << debugString() << "WARNING: Invalid parameter index.";
        return EffectParameterPointer();
    }
    return m_parameters[parameterNumber];
}
