#include "controleffectknob.h"
#include "defs.h"

#include "effects/effectmanifestparameter.h"

ControlEffectKnob::ControlEffectKnob(ConfigKey key, double dMinValue, double dMaxValue)
        : ControlPotmeter(key, dMinValue, dMaxValue),
          m_type(0.0) {
    if (m_pControl) {
        m_pControl->setBehavior(
                new ControlLinPotmeterBehavior(dMinValue, dMaxValue));
    }
}

void ControlEffectKnob::setType(double v) {
    if (v == m_type || m_pControl == NULL) {
        return;
    }

    if (v == EffectManifestParameter::CONTROL_KNOB_LINEAR) {
        m_pControl->setBehavior(
                        new ControlLinPotmeterBehavior(m_dMinValue, m_dMaxValue));
    } else if (v == EffectManifestParameter::CONTROL_KNOB_LOGARITHMIC) {
        m_pControl->setBehavior(
                        new ControlLogPotmeterBehavior(m_dMinValue, m_dMaxValue));
    }
    m_type = v;
}
