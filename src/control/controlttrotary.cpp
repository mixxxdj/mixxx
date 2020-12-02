#include "control/controlttrotary.h"

/* -------- ------------------------------------------------------
   Purpose: Creates a new rotary encoder
   Input:   key
   -------- ------------------------------------------------------ */
ControlTTRotary::ControlTTRotary(const ConfigKey& key)
        : ControlObject(key) {
    if (m_pControl) {
        m_pControl->setBehavior(
                new ControlTTRotaryBehavior());
    }
}
