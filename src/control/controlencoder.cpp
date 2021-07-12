#include "control/controlencoder.h"

#include "moc_controlencoder.cpp"

ControlEncoder::ControlEncoder(const ConfigKey& key, bool bIgnoreNops)
        : ControlObject(key, bIgnoreNops) {
    if (m_pControl) {
        m_pControl->setBehavior(new ControlEncoderBehavior());
    }
}
