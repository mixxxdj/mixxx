#include "control/controleffectknob.h"

#include "effects/effectmanifestparameter.h"
#include "moc_controleffectknob.cpp"
#include "util/math.h"

ControlEffectKnob::ControlEffectKnob(const ConfigKey& key, double dMinValue, double dMaxValue)
        : ControlPotmeter(key, dMinValue, dMaxValue) {
}

void ControlEffectKnob::setBehaviour(EffectManifestParameter::ControlHint type,
                                     double dMinValue, double dMaxValue) {
    if (m_pControl == nullptr) {
        return;
    }

    if (type == EffectManifestParameter::ControlHint::KNOB_LINEAR) {
            m_pControl->setBehavior(new ControlLinPotmeterBehavior(
                    dMinValue, dMaxValue, false));
    } else if (type == EffectManifestParameter::ControlHint::KNOB_LINEAR_INVERSE) {
            m_pControl->setBehavior(new ControlLinInvPotmeterBehavior(
                    dMinValue, dMaxValue, false));
    } else if (type == EffectManifestParameter::ControlHint::KNOB_LOGARITHMIC) {
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
    } else if (type == EffectManifestParameter::ControlHint::KNOB_LOGARITHMIC_INVERSE) {
        m_pControl->setBehavior(
                new ControlLogInvPotmeterBehavior(dMinValue, dMaxValue, -40));
    }
}
