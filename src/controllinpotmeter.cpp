#include "controllinpotmeter.h"
#include "defs.h"

ControlLinPotmeter::ControlLinPotmeter(ConfigKey key, double dMinValue, double dMaxValue) :
    ControlPotmeter(key, dMinValue, dMaxValue) {
    if (m_pControl) {
        m_pControl->setBehavior(
                new ControlLinPotmeterBehavior(dMinValue, dMaxValue));
    }
}
