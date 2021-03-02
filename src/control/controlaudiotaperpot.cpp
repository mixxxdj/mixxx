#include "control/controlaudiotaperpot.h"

#include "moc_controlaudiotaperpot.cpp"

ControlAudioTaperPot::ControlAudioTaperPot(const ConfigKey& key,
        double minDB,
        double maxDB,
        double neutralParameter)
        : ControlPotmeter(key) {
    // Override ControlPotmeters default value of 0.5
    setDefaultValue(1.0);
    set(1.0);

    if (m_pControl) {
        m_pControl->setBehavior(
                new ControlAudioTaperPotBehavior(minDB, maxDB,
                        neutralParameter));
    }
}
