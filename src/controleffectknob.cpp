#include "controleffectknob.h"

#include "effects/effectmanifestparameter.h"

ControlEffectKnob::ControlEffectKnob(ConfigKey key, double dMinValue, double dMaxValue)
        : ControlPotmeter(key, dMinValue, dMaxValue) {
}

void ControlEffectKnob::setType(EffectManifestParameter::ControlHint type) {
    if ( m_pControl == NULL) {
        return;
    }

    if (type == EffectManifestParameter::CONTROL_KNOB_LINEAR) {
            m_pControl->setBehavior(new ControlLinPotmeterBehavior(
                    m_dMinValue, m_dMaxValue, false));
    } else if (type == EffectManifestParameter::CONTROL_KNOB_LOGARITHMIC) {
            m_pControl->setBehavior(new ControlLogPotmeterBehavior(
                    m_dMinValue, m_dMaxValue));
    }
}
