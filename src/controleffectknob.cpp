#include "controleffectknob.h"

#include "effects/effectmanifestparameter.h"

ControlEffectKnob::ControlEffectKnob(ConfigKey key, double dMinValue, double dMaxValue)
        : ControlPotmeter(key, dMinValue, dMaxValue) {
}

void ControlEffectKnob::setBehaviour(EffectManifestParameter::ControlHint type,
                                     double dMinValue, double dMaxValue) {
    if ( m_pControl == NULL) {
        return;
    }

    if (type == EffectManifestParameter::CONTROL_KNOB_LINEAR) {
            m_pControl->setBehavior(new ControlLinPotmeterBehavior(
                    dMinValue, dMaxValue, false));
    } else if (type == EffectManifestParameter::CONTROL_KNOB_LOGARITHMIC) {
            m_pControl->setBehavior(new ControlLogPotmeterBehavior(
                    dMinValue, dMaxValue));
    }
}
