#include "control/controlencoder.h"

ControlEncoder::ControlEncoder(const ConfigKey& key, bool bIgnoreNops)
        : ControlObject(key, bIgnoreNops) {
    if (m_pControl) {
        m_pControl->setBehavior(new ControlEncoderBehavior());
    }
}
