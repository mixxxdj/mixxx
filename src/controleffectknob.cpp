#include "controleffectknob.h"

#include "util/math.h"
#include "effects/effectmanifestparameter.h"

ControlEffectKnob::ControlEffectKnob(ConfigKey key, EffectKnobParameters parameters)
        : ControlPotmeter(key, parameters) {
}

void ControlEffectKnob::setBehavior(EffectManifestParameter::ControlHint type,
                                     double dMinValue, double dMaxValue,
                                     double dNeutralParameter) {
    if (m_pControl == NULL) {
        return;
    }

    if (type == EffectManifestParameter::CONTROL_KNOB_LINEAR) {
            m_pControl->setBehavior(new ControlLinPotmeterBehavior(
                    dMinValue, dMaxValue, dNeutralParameter, false));
    } else if (type == EffectManifestParameter::CONTROL_KNOB_LOGARITHMIC) {
        if (dMinValue == 0) {
            if (dMaxValue == 1.0) {
                // Volume like control
                m_pControl->setBehavior(
                        new ControlAudioTaperPotBehavior(-20, 0, dNeutralParameter));
            } else if (dMaxValue > 1.0) {
                // Gain like control
                m_pControl->setBehavior(
                        new ControlAudioTaperPotBehavior(-12, ratio2db(dMaxValue), dNeutralParameter));
            } else {
                m_pControl->setBehavior(
                        new ControlLogPotmeterBehavior(dMinValue, dMaxValue, dNeutralParameter, -40));
            }
        } else {
            m_pControl->setBehavior(
                    new ControlLogPotmeterBehavior(dMinValue, dMaxValue, dNeutralParameter, -40));
        }
    }
}
