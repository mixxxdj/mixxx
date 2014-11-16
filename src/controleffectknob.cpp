#include "controleffectknob.h"

#include "util/math.h"
#include "effects/effectmanifestparameter.h"

ControlEffectKnob::ControlEffectKnob(ConfigKey key, double dMinValue, double dMaxValue)
        : ControlPotmeter(key, dMinValue, dMaxValue) {
}

void ControlEffectKnob::setBehaviour(EffectManifestParameter::ControlHint type,
                                     double dMinValue, double dMaxValue) {
    if (m_pControl == NULL) {
        return;
    }

    if (type == EffectManifestParameter::CONTROL_KNOB_LINEAR) {
            m_pControl->setBehavior(new ControlLinPotmeterBehavior(
                    dMinValue, dMaxValue, false));
    } else if (type == EffectManifestParameter::CONTROL_KNOB_LOGARITHMIC) {
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
    }
}
