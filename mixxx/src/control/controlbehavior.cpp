#include "control/controlbehavior.h"
#include "control/control.h"

// static
const int ControlPushButtonBehavior::kPowerWindowTimeMillis = 300;

void ControlNumericBehavior::setValueFromMidiParameter(MidiOpCode o, double dParam,
                                                       ControlDoublePrivate* pControl) {
    Q_UNUSED(o);
    pControl->set(dParam, NULL);
}

void ControlPotmeterBehavior::setValueFromMidiParameter(MidiOpCode o, double dParam,
                                                        ControlDoublePrivate* pControl) {
    Q_UNUSED(o);
    pControl->set(widgetParameterToValue(dParam), NULL);
}

void ControlPushButtonBehavior::setValueFromMidiParameter(
    MidiOpCode o, double dParam, ControlDoublePrivate* pControl) {
    // This block makes push-buttons act as power window buttons.
    if (m_buttonMode == POWERWINDOW && m_iNumStates == 2) {
        if (o == MIDI_NOTE_ON) {
            if (dParam > 0.) {
                double value = pControl->get();
                pControl->set(!value, NULL);
                m_pushTimer.setSingleShot(true);
                m_pushTimer.start(kPowerWindowTimeMillis);
            }
        } else if (o == MIDI_NOTE_OFF) {
            if (!m_pushTimer.isActive()) {
                pControl->set(0.0, NULL);
            }
        }
    } else if (m_buttonMode == TOGGLE) {
        // This block makes push-buttons act as toggle buttons.
        if (m_iNumStates > 2) { //multistate button
            if (dParam > 0.) { //looking for NOTE_ON doesn't seem to work...
                double value = pControl->get();
                value++;
                if (value >= m_iNumStates) {
                    pControl->set(0, NULL);
                } else {
                    pControl->set(value, NULL);
                }
            }
        } else {
            if (o == MIDI_NOTE_ON) {
                if (dParam > 0.) {
                    double value = pControl->get();
                    pControl->set(!value, NULL);
                }
            }
        }
    } else { //Not a toggle button (trigger only when button pushed)
        if (o == MIDI_NOTE_ON) {
            double value = pControl->get();
            pControl->set(!value, NULL);
        } else if (o == MIDI_NOTE_OFF) {
            pControl->set(0.0, NULL);
        }
    }
}
