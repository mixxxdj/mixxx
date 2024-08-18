#include "control/controlencoder.h"

#include "control/control.h"
#include "moc_controlencoder.cpp"

ControlEncoder::ControlEncoder(const ConfigKey& key, bool bIgnoreNops)
        : ControlObject(key,
                  bIgnoreNops ? ControlConfigFlag::IgnoreNops
                              : ControlConfigFlag::None) {
    if (m_pControl) {
        m_pControl->setBehavior(new ControlEncoderBehavior());
    }
}
