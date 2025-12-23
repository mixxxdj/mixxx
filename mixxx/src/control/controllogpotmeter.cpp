#include "control/controllogpotmeter.h"

#include "moc_controllogpotmeter.cpp"

ControlLogpotmeter::ControlLogpotmeter(const ConfigKey& key, double dMaxValue, double minDB)
        : ControlPotmeter(key, 0, dMaxValue) {
    // Override ControlPotmeters default value of 0.5
    setDefaultValue(1.0);
    set(1.0);

    if (m_pControl) {
        m_pControl->setBehavior(
                new ControlLogPotmeterBehavior(0, dMaxValue, minDB));
    }
}
