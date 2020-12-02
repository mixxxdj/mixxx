#include "control/controlencoder.h"

#include <QSharedPointer>

#include "control/control.h"
#include "control/controlbehavior.h"

class ConfigKey;

ControlEncoder::ControlEncoder(const ConfigKey& key, bool bIgnoreNops)
        : ControlObject(key, bIgnoreNops) {
    if (m_pControl) {
        m_pControl->setBehavior(new ControlEncoderBehavior());
    }
}
