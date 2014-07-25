
#include "controlaudiotaperpot.h"

// The AudioTaperPot has a log scale, starting at -Invinity
// minDB is the Start value of the pure db scale it cranked to -Infinity by the linar part of the AudioTaperPot
// maxDB is the Upper gain Value
// neutralParameter is a knob position between 0 and 1 where the gain is 1 (0dB)
ControlAudioTaperPot::ControlAudioTaperPot(ConfigKey key,
                                           double minDB, double maxDB,
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
