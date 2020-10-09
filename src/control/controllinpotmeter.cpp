#include "control/controllinpotmeter.h"

ControlLinPotmeter::ControlLinPotmeter(ConfigKey key,
                                       double dMinValue, double dMaxValue,
                                       double dStep, double dSmallStep,
                                       bool allowOutOfBounds)
        : ControlPotmeter(key, dMinValue, dMaxValue, allowOutOfBounds) {
    if (m_pControl) {
        m_pControl->setBehavior(
            new ControlLinPotmeterBehavior(dMinValue, dMaxValue, allowOutOfBounds));
    }
    if (dStep) {
        setStepCount(static_cast<int>((dMaxValue - dMinValue) / dStep));
    }
    if (dSmallStep) {
        setSmallStepCount(static_cast<int>((dMaxValue - dMinValue) / dSmallStep));
    }
}
