#include "control/controlbehavior.h"
#include "control/control.h"
#include "util/math.h"

bool ControlNumericBehavior::setFilter(double* dValue) {
    Q_UNUSED(dValue);
    return true;
}

double ControlNumericBehavior::defaultValue(double dDefault) const {
    return dDefault;
}

double ControlNumericBehavior::valueToParameter(double dValue) {
    return dValue;
}

double ControlNumericBehavior::midiValueToParameter(double midiValue) {
    return midiValue;
}

double ControlNumericBehavior::parameterToValue(double dParam) {
    return dParam;
}

double ControlNumericBehavior::valueToMidiParameter(double dValue) {
    return dValue;
}

void ControlNumericBehavior::setValueFromMidiParameter(MidiOpCode o, double dParam,
                                                       ControlDoublePrivate* pControl) {
    Q_UNUSED(o);
    double dNorm = midiValueToParameter(dParam);
    pControl->set(parameterToValue(dNorm), NULL);
}

ControlPotmeterBehavior::ControlPotmeterBehavior(double dMinValue, double dMaxValue,
                                                 bool allowOutOfBounds)
        : m_dMinValue(dMinValue),
          m_dMaxValue(dMaxValue),
          m_dValueRange(m_dMaxValue - m_dMinValue),
          m_dDefaultValue(m_dMinValue + 0.5 * m_dValueRange),
          m_bAllowOutOfBounds(allowOutOfBounds) {
}

ControlPotmeterBehavior::~ControlPotmeterBehavior() {
}

bool ControlPotmeterBehavior::setFilter(double* dValue) {
    if (!m_bAllowOutOfBounds) {
        if (*dValue > m_dMaxValue) {
            *dValue = m_dMaxValue;
        } else if (*dValue < m_dMinValue) {
            *dValue = m_dMinValue;
        }
    }
    return true;
}

double ControlPotmeterBehavior::defaultValue(double dDefault) const {
    Q_UNUSED(dDefault);
    return m_dDefaultValue;
}

double ControlPotmeterBehavior::valueToParameter(double dValue) {
    if (m_dValueRange == 0.0) {
        return 0;
    }
    if (dValue > m_dMaxValue) {
        dValue = m_dMaxValue;
    } else if (dValue < m_dMinValue) {
        dValue = m_dMinValue;
    }
    return (dValue - m_dMinValue) / m_dValueRange;
}

double ControlPotmeterBehavior::midiValueToParameter(double midiValue) {
    double parameter;
    if (midiValue > 64) {
        parameter = (midiValue - 1) / 126.0;
    } else {
        // Hack for 0.5 at 64
        parameter = midiValue / 128.0;
    }
    return parameter;
}

double ControlPotmeterBehavior::parameterToValue(double dParam) {
    return m_dMinValue + (dParam * m_dValueRange);
}

double ControlPotmeterBehavior::valueToMidiParameter(double dValue) {
    // 7-bit MIDI has 128 values [0, 127]. This means there is no such thing as
    // center. The industry convention is that 64 is center. We fake things a
    // little bit here to make that the case. This function is linear from [0,
    // 127.0/128.0] with slope 128 and then cuts off at 127 from 127.0/128.0 to
    // 1.0.  from 0 to 64 with slope 128 and from 64 to 127 with slope 126.
    double dNorm = valueToParameter(dValue);
    if (dNorm > 0.5) {
        return (dNorm * 126) + 1;
    } else {
        return dNorm * 128.0;
    }
}

#define maxPosition 1.0
#define minPosition 0.0
#define middlePosition ((maxPosition - minPosition) / 2.0)
#define positionrange (maxPosition - minPosition)

ControlLogPotmeterBehavior::ControlLogPotmeterBehavior(double dMinValue, double dMaxValue)
        : ControlPotmeterBehavior(dMinValue, dMaxValue, false),
          m_dB1(0.0),
          m_dB2(0.0) {
    if (dMaxValue == 1.0 || dMinValue != 0.0 ) {
        m_bTwoState = false;
        m_dB1 = log10((dMaxValue - dMinValue) + 1.0) / maxPosition;
    } else {
        m_bTwoState = true;
        m_dB1 = log10(2.0) / middlePosition;
        m_dB2 = log10(dMaxValue) / (maxPosition - middlePosition);
    }
}

ControlLogPotmeterBehavior::~ControlLogPotmeterBehavior() {
}

double ControlLogPotmeterBehavior::defaultValue(double dDefault) const {
    Q_UNUSED(dDefault);
    return 1.0;
}

double ControlLogPotmeterBehavior::valueToParameter(double dValue) {
    if (dValue > m_dMaxValue) {
        dValue = m_dMaxValue;
    } else if (dValue < m_dMinValue) {
        dValue = m_dMinValue;
    }
    if (!m_bTwoState) {
        return log10((dValue - m_dMinValue) + 1) / m_dB1;
    } else {
        if (dValue > 1.0) {
            return log10(dValue) / m_dB2 + middlePosition;
        } else {
            return log10(dValue + 1.0) / m_dB1;
        }
    }
}

double ControlLogPotmeterBehavior::parameterToValue(double dParam) {
    if (!m_bTwoState) {
        return pow(10.0, m_dB1 * dParam) - 1.0 + m_dMinValue;
    } else {
        if (dParam <= middlePosition) {
            return pow(10.0, m_dB1 * dParam) - 1;
        } else {
            return pow(10.0, m_dB2 * (dParam - middlePosition));
        }
    }
}

ControlLinPotmeterBehavior::ControlLinPotmeterBehavior(double dMinValue, double dMaxValue,
                                                       bool allowOutOfBounds)
        : ControlPotmeterBehavior(dMinValue, dMaxValue, allowOutOfBounds) {
}

ControlLinPotmeterBehavior::~ControlLinPotmeterBehavior() {
}

double ControlTTRotaryBehavior::valueToParameter(double dValue) {
    return (dValue * 200.0 + 64) / 127.0;
}

double ControlTTRotaryBehavior::parameterToValue(double dParam) {
    dParam *= 128.0;
    // Non-linear scaling
    double temp = ((dParam - 64.0) * (dParam - 64.0)) / 64.0;
    if (dParam - 64 < 0) {
        temp = -temp;
    }
    return temp;
}

// static
const int ControlPushButtonBehavior::kPowerWindowTimeMillis = 300;
const int ControlPushButtonBehavior::kLongPressLatchingTimeMillis = 300;

ControlPushButtonBehavior::ControlPushButtonBehavior(ButtonMode buttonMode,
                                                     int iNumStates)
        : m_buttonMode(buttonMode),
          m_iNumStates(iNumStates) {
}

void ControlPushButtonBehavior::setValueFromMidiParameter(
        MidiOpCode o, double dParam, ControlDoublePrivate* pControl) {
    // Calculate pressed State of the midi Button
    // Some controller like the RMX2 are sending always MIDI_NOTE_ON
    // with a changed dParam 127 for pressed an 0 for released.
    // Other controller like the VMS4 are using MIDI_NOTE_ON
    // And MIDI_NOTE_OFF and a velocity value like a piano keyboard
    bool pressed = true;
    if (o == MIDI_NOTE_OFF || dParam == 0) {
        // MIDI_NOTE_ON + 0 should be interpreted a released according to
        // http://de.wikipedia.org/wiki/Musical_Instrument_Digital_Interface
        // looking for MIDI_NOTE_ON doesn't seem to work...
        pressed = false;
    }

    // This block makes push-buttons act as power window buttons.
    if (m_buttonMode == POWERWINDOW && m_iNumStates == 2) {
        if (pressed) {
            // Toggle on press
            double value = pControl->get();
            pControl->set(!value, NULL);
            m_pushTimer.setSingleShot(true);
            m_pushTimer.start(kPowerWindowTimeMillis);
        } else if (!m_pushTimer.isActive()) {
            // Disable after releasing a long press
            pControl->set(0., NULL);
        }
    } else if (m_buttonMode == TOGGLE || m_buttonMode == LONGPRESSLATCHING) {
        // This block makes push-buttons act as toggle buttons.
        if (m_iNumStates > 1) { // multistate button
            if (pressed) {
                // This is a possibly race condition if another writer wants
                // to change the value at the same time. We allow the race here,
                // because this is possibly what the user expects if he changes
                // the same control from different devices.
                double value = pControl->get();
                value = (int)(value + 1.) % m_iNumStates;
                pControl->set(value, NULL);
                if (m_buttonMode == LONGPRESSLATCHING) {
                    m_pushTimer.setSingleShot(true);
                    m_pushTimer.start(kLongPressLatchingTimeMillis);
                }
            } else {
                double value = pControl->get();
                if (m_buttonMode == LONGPRESSLATCHING &&
                        m_pushTimer.isActive() && value >= 1.) {
                    // revert toggle if button is released too early
                    value = (int)(value - 1.) % m_iNumStates;
                    pControl->set(value, NULL);
                }
            }
        }
    } else { // Not a toggle button (trigger only when button pushed)
        if (pressed) {
            pControl->set(1., NULL);
        } else {
            pControl->set(0., NULL);
        }
    }
}
