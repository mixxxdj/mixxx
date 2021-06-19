#include "control/controllinpotmeter.h"

#include "moc_controllinpotmeter.cpp"

ControlLinPotmeter::ControlLinPotmeter(const ConfigKey& key,
        double dMinValue,
        double dMaxValue,
        double dStep,
        double dSmallStep,
        bool allowOutOfBounds)
        : ControlPotmeter(key, dMinValue, dMaxValue, allowOutOfBounds) {
    if (m_pControl) {
        m_pControl->setBehavior(
            new ControlLinPotmeterBehavior(dMinValue, dMaxValue, allowOutOfBounds));
    }
    if (dStep != 0) {
        setStepCount(static_cast<int>((dMaxValue - dMinValue) / dStep));
    }
    if (dSmallStep != 0) {
        setSmallStepCount(static_cast<int>((dMaxValue - dMinValue) / dSmallStep));
    }
}
