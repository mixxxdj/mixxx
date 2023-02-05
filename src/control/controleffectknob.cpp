#include "control/controleffectknob.h"

#include "effects/backends/effectmanifestparameter.h"
#include "moc_controleffectknob.cpp"
#include "util/math.h"

ControlEffectKnob::ControlEffectKnob(const ConfigKey& key, double dMinValue, double dMaxValue)
        : ControlPotmeter(key, dMinValue, dMaxValue) {
}

void ControlEffectKnob::setBehaviour(EffectManifestParameter::ValueScaler type,
        double dMinValue,
        double dMaxValue) {
    if (m_pControl == nullptr) {
        return;
    }

    if (type == EffectManifestParameter::ValueScaler::Linear) {
        m_pControl->setBehavior(new ControlLinPotmeterBehavior(
                dMinValue, dMaxValue, false));
    } else if (type == EffectManifestParameter::ValueScaler::LinearInverse) {
        m_pControl->setBehavior(new ControlLinInvPotmeterBehavior(
                dMinValue, dMaxValue, false));
    } else if (type == EffectManifestParameter::ValueScaler::Logarithmic) {
        if (dMinValue == 0) {
            if (dMaxValue == 1.0) {
                // Volume like control
                m_pControl->setBehavior(
                        new ControlAudioTaperPotBehavior(-20, 0, 1));
            } else if (dMaxValue > 1.0) {
                // Gain like control
                m_pControl->setBehavior(
                        new ControlAudioTaperPotBehavior(-12, ratio2db(dMaxValue), 0.5));
            } else {
                m_pControl->setBehavior(
                        new ControlLogPotmeterBehavior(dMinValue, dMaxValue, -40));
            }
        } else {
            m_pControl->setBehavior(
                    new ControlLogPotmeterBehavior(dMinValue, dMaxValue, -40));
        }
    } else if (type == EffectManifestParameter::ValueScaler::LogarithmicInverse) {
        m_pControl->setBehavior(
                new ControlLogInvPotmeterBehavior(dMinValue, dMaxValue, -40));
    } else if (type == EffectManifestParameter::ValueScaler::Integral) {
        m_pControl->setBehavior(new ControlLinSteppedIntPotBehavior(
                dMinValue, dMaxValue, false));
    }
}
