#include "controleffectknob.h"
#include "defs.h"

#include "effects/effectmanifestparameter.h"

ControlEffectKnob::ControlEffectKnob(ConfigKey key, double dMinValue, double dMaxValue)
        : ControlPotmeter(key, dMinValue, dMaxValue),
          m_type(EffectManifestParameter::CONTROL_KNOB_LINEAR) {
    if (m_pControl) {
        m_pControl->setBehavior(
                new ControlLinPotmeterBehavior(dMinValue, dMaxValue, false));
    }
}

void ControlEffectKnob::setType(EffectManifestParameter::ControlHint type) {
    if (type == m_type || m_pControl == NULL) {
        return;
    }

    if (type == EffectManifestParameter::CONTROL_KNOB_LINEAR) {
        m_pControl->setBehavior(
                new ControlLinPotmeterBehavior(m_dMinValue, m_dMaxValue, false));
    } else if (type == EffectManifestParameter::CONTROL_KNOB_LOGARITHMIC) {
        m_pControl->setBehavior(
                new ControlLogPotmeterBehavior(m_dMinValue, m_dMaxValue));
    }
    m_type = type;
}
