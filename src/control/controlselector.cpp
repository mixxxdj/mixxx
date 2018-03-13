#include "control/controlselector.h"

ControlSelector::ControlSelector(ConfigKey key, bool bIgnoreNops)
        : ControlObject(key, bIgnoreNops) {
    if (m_pControl) {
        m_pControl->setBehavior(new ControlSelectBehavior());
    }
}
