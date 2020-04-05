#include <QtDebug>

#include "effects/effectparameter.h"
#include "effects/effectsmanager.h"
#include "util/assert.h"

EffectParameter::EffectParameter(EngineEffect* pEngineEffect,
        EffectsManager* pEffectsManager,
        EffectManifestParameterPointer pParameter,
        EffectParameterPreset preset)
        : m_pEngineEffect(pEngineEffect),
          m_pEffectsManager(pEffectsManager),
          m_pParameter(pParameter) {
    if (preset.isNull()) {
        setValue(pParameter->getDefault());
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
    return m_pParameter;
}

// static
bool EffectParameter::clampValue(double* pValue,
                                 const double& minimum, const double& maximum) {
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
    return clampValue(&m_value, m_pParameter->getMinimum(), m_pParameter->getMaximum());
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
    pRequest->SetParameterParameters.iParameter = m_pParameter->index();
    pRequest->value = m_value;
    pRequest->minimum = m_pParameter->getMinimum();
    pRequest->maximum = m_pParameter->getMaximum();
    pRequest->default_value = m_pParameter->getDefault();
    m_pEffectsManager->writeRequest(pRequest);
}
