#include "control/controlbehavior.h"
#include "control/control.h"

// static
const int ControlPushButtonBehavior::kPowerWindowTimeMillis = 300;

void ControlNumericBehavior::setValueFromMidiParameter(MidiOpCode o, double dParam,
                                                       ControlDoublePrivate* pControl,
                                                       QObject* pSender) {
    Q_UNUSED(o);
    pControl->set(dParam, pSender);
}

void ControlPotmeterBehavior::setValueFromMidiParameter(MidiOpCode o, double dParam,
                                                        ControlDoublePrivate* pControl,
                                                        QObject* pSender) {
    Q_UNUSED(o);
    pControl->set(widgetParameterToValue(dParam), pSender);
}

void ControlPushButtonBehavior::setValueFromMidiParameter(
        MidiOpCode o, double dParam, ControlDoublePrivate* pControl, QObject* pSender) {
    // This block makes push-buttons act as power window buttons.
    if (m_buttonMode == POWERWINDOW && m_iNumStates == 2) {
        if (o == MIDI_NOTE_ON) {
            if (dParam > 0.) {
                double value = pControl->get();
                pControl->set(!value, pSender);
                m_pushTimer.setSingleShot(true);
                m_pushTimer.start(kPowerWindowTimeMillis);
            }
        } else if (o == MIDI_NOTE_OFF) {
            if (!m_pushTimer.isActive()) {
                pControl->set(0.0, pSender);
            }
        }
    } else if (m_buttonMode == TOGGLE) {
        // This block makes push-buttons act as toggle buttons.
        if (m_iNumStates > 2) { // multistate button
            if (dParam > 0.) { // looking for NOTE_ON doesn't seem to work...
                // This is a possible race condition if an other writer wants
                // to change the value at the same time. We allow the race her,
                // because this is possible what the user expect if he controls
                // the same control from different devices.
                double value = pControl->get();
                value++;
                if (value >= m_iNumStates) {
                    pControl->set(0, pSender);
                } else {
                    pControl->set(value, pSender);
                }
            }
        } else {
            if (o == MIDI_NOTE_ON) {
                if (dParam > 0.) {
                    double value = pControl->get();
                    pControl->set(!value, pSender);
                }
            }
        }
    } else { //Not a toggle button (trigger only when button pushed)
        if (o == MIDI_NOTE_ON) {
            double value = pControl->get();
            pControl->set(!value, pSender);
        } else if (o == MIDI_NOTE_OFF) {
            pControl->set(0.0, pSender);
        }
    }
}
