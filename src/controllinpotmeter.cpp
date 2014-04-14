#include "controllinpotmeter.h"
#include "defs.h"

ControlLinPotmeter::ControlLinPotmeter(ConfigKey key,
                                       double dMinValue, double dMaxValue,
                                       bool allowOutOfBounds)
        : ControlPotmeter(key, dMinValue, dMaxValue, allowOutOfBounds) {
    if (m_pControl) {
        m_pControl->setBehavior(
            new ControlLinPotmeterBehavior(dMinValue, dMaxValue, allowOutOfBounds));
    }
}
