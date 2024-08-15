#include "control/controlencoder.h"

#include "control/control.h"
#include "moc_controlencoder.cpp"

using enum ControlConfigFlag;

ControlEncoder::ControlEncoder(const ConfigKey& key, bool bIgnoreNops)
        : ControlObject(key, ControlConfigFlags(Default).setFlag(IgnoreNops, bIgnoreNops)) {
    if (m_pControl) {
        m_pControl->setBehavior(new ControlEncoderBehavior());
    }
}
