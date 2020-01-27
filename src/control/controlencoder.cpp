#include "control/controlencoder.h"

ControlEncoder::ControlEncoder(ConfigKey key, bool bIgnoreNops)
        : ControlObject(key, bIgnoreNops) {
    if (m_pControl) {
        m_pControl->setBehavior(new ControlEncoderBehavior());
    }
}
