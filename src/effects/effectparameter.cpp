#include "effects/effectparameter.h"

#include <QtDebug>

#include "effects/effectsmessenger.h"
#include "effects/presets/effectparameterpreset.h"

EffectParameter::EffectParameter(EngineEffect* pEngineEffect,
        EffectsMessengerPointer pEffectsMessenger,
        EffectManifestParameterPointer pParameterManifest,
        const EffectParameterPreset& preset)
        : m_pEngineEffect(pEngineEffect),
          m_pMessenger(pEffectsMessenger),
          m_pParameterManifest(pParameterManifest) {
    if (preset.isNull()) {
        setValue(pParameterManifest->getDefault());
    } else {
        setValue(preset.value());
        m_linkType = preset.linkType();
        m_linkInversion = preset.linkInverted();
    }
}

EffectParameter::~EffectParameter() {
    //qDebug() << debugString() << "destroyed";
}

EffectManifestParameterPointer EffectParameter::manifest() const {
    return m_pParameterManifest;
}

// static
bool EffectParameter::clampValue(double* pValue,
        const double& minimum,
        const double& maximum) {
    if (*pValue < minimum) {
        *pValue = minimum;
        return true;
    } else if (*pValue > maximum) {
        *pValue = maximum;
        return true;
    }
    return false;
}

bool EffectParameter::clampValue() {
    return clampValue(&m_value,
            m_pParameterManifest->getMinimum(),
            m_pParameterManifest->getMaximum());
}

double EffectParameter::getValue() const {
    return m_value;
}

void EffectParameter::setValue(double value) {
    // TODO(XXX) Handle inf, -inf, and nan
    m_value = value;

    if (clampValue()) {
        qWarning() << debugString() << "WARNING: Value was outside of limits, clamped.";
    }

    updateEngineState();
}

void EffectParameter::updateEngineState() {
    if (!m_pEngineEffect) {
        return;
    }
    EffectsRequest* pRequest = new EffectsRequest();
    pRequest->type = EffectsRequest::SET_PARAMETER_PARAMETERS;
    pRequest->pTargetEffect = m_pEngineEffect;
    pRequest->SetParameterParameters.iParameter = m_pParameterManifest->index();
    pRequest->value = m_value;
    m_pMessenger->writeRequest(pRequest);
}
