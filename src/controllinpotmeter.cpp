#include "controllinpotmeter.h"

ControlLinPotmeter::ControlLinPotmeter(ConfigKey key,
                                       double dMinValue, double dMaxValue,
                                       double dStep, double dSmallStep,
                                       double dNeutralValue, bool allowOutOfBounds)
        : ControlPotmeter(key, dMinValue, dMaxValue, dNeutralValue, allowOutOfBounds) {
    if (m_pControl) {
        m_pControl->setBehavior(
            new ControlLinPotmeterBehavior(dMinValue, dMaxValue, dNeutralValue, allowOutOfBounds));
    }
    if (dStep) {
        setStepCount((dMaxValue - dMinValue) / dStep);
    }
    if (dSmallStep) {
        setSmallStepCount((dMaxValue - dMinValue) / dSmallStep);
    }
}



