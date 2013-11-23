#include "controllinpotmeter.h"
#include "defs.h"

// This control has a linear link between the m_dValue and the Midi Value
// limitation: m_dMaxValue represents the midi value of 128 and is never reached
ControlLinPotmeter::ControlLinPotmeter(ConfigKey key, double dMinValue, double dMaxValue) :
    ControlPotmeter(key, dMinValue, dMaxValue) {
    if (m_pControl) {
        ControlNumericBehavior* pOldBehavior = m_pControl->setBehavior(
            new ControlLinPotmeterBehavior(dMinValue, dMaxValue));
        delete pOldBehavior;
    }
}


