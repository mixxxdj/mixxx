#include "controleffectknob.h"

#include "util/math.h"
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
        if (m_dMinValue == 0) {
            if (m_dMaxValue == 1.0) {
                // Volume like control
                m_pControl->setBehavior(
                        new ControlAudioTaperPotBehavior(-20, 0, 1));
            } else {
                // Gain like control
                m_pControl->setBehavior(
                        new ControlAudioTaperPotBehavior(-12, ratio2db(m_dMaxValue), 0.5));
            }
        } else {
            m_pControl->setBehavior(
                    new ControlLogPotmeterBehavior(m_dMinValue, m_dMaxValue));
        }
    }
    m_type = type;
}
