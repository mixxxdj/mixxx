#include <QtDebug>

#include "effects/effectparameter.h"
#include "effects/effectsmanager.h"
#include "util/assert.h"

EffectParameter::EffectParameter(EffectSlot* pEffectSlot, EffectsManager* pEffectsManager,
                                 int iParameterNumber, EffectManifestParameterPointer pParameter)
        : QObject(), // no parent
          m_pEffectSlot(pEffectSlot),
          m_pEffectsManager(pEffectsManager),
          m_iParameterNumber(iParameterNumber),
          m_pParameter(pParameter) {
    // qDebug() << debugString() << "Constructing new EffectParameter from EffectManifestParameter:"
    //          << m_parameter.id();

    // Set the value to the default.
    m_value = m_pParameter->getDefault();
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

    m_value = value;

    updateEngineState();
    emit(valueChanged(m_value));
}

void EffectParameter::updateEngineState() {
    EngineEffect* pEngineEffect = m_pEffectSlot->getEngineEffect();
    if (!pEngineEffect) {
        return;
    }
    EffectsRequest* pRequest = new EffectsRequest();
    pRequest->type = EffectsRequest::SET_PARAMETER_PARAMETERS;
    pRequest->pTargetEffect = pEngineEffect;
    pRequest->SetParameterParameters.iParameter = m_iParameterNumber;
    pRequest->value = m_value;
    pRequest->minimum = m_pParameter->getMinimum();
    pRequest->maximum = m_pParameter->getMaximum();
    pRequest->default_value = m_pParameter->getDefault();
    m_pEffectsManager->writeRequest(pRequest);
}
