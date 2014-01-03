#include "control/controlbehavior.h"
#include "control/control.h"

bool ControlNumericBehavior::setFilter(double* dValue) {
    Q_UNUSED(dValue);
    return true;
}

double ControlNumericBehavior::defaultValue(double dDefault) const {
    return dDefault;
}

double ControlNumericBehavior::valueToWidgetParameter(double dValue) {
    return dValue;
}

double ControlNumericBehavior::widgetParameterToValue(double dParam) {
    return dParam;
}

double ControlNumericBehavior::valueToMidiParameter(double dValue) {
    return dValue;
}

void ControlNumericBehavior::setValueFromMidiParameter(MidiOpCode o, double dParam,
                                                       ControlDoublePrivate* pControl) {
    Q_UNUSED(o);
    pControl->set(dParam, NULL);
}

ControlPotmeterBehavior::ControlPotmeterBehavior(double dMinValue, double dMaxValue)
        : m_dMinValue(dMinValue),
          m_dMaxValue(dMaxValue),
          m_dValueRange(m_dMaxValue - m_dMinValue),
          m_dDefaultValue(m_dMinValue + 0.5 * m_dValueRange) {
}

ControlPotmeterBehavior::~ControlPotmeterBehavior() {
}

bool ControlPotmeterBehavior::setFilter(double* dValue) {
    Q_UNUSED(dValue);
    return true;
}

double ControlPotmeterBehavior::defaultValue(double dDefault) const {
    Q_UNUSED(dDefault);
    return m_dDefaultValue;
}

double ControlPotmeterBehavior::valueToWidgetParameter(double dValue) {
    if (dValue > m_dMaxValue) {
        dValue = m_dMaxValue;
    } else if (dValue < m_dMinValue) {
        dValue = m_dMinValue;
    }
    if (m_dValueRange == 0.0) {
        return 0;
    }
    return (dValue - m_dMinValue) / m_dValueRange;
}

double ControlPotmeterBehavior::widgetParameterToValue(double dParam) {
    return m_dMinValue + dParam * m_dValueRange;
}

double ControlPotmeterBehavior::valueToMidiParameter(double dValue) {
    // 7-bit MIDI has 128 values [0, 127]. This means there is no such thing as
    // center. The industry convention is that 64 is center. We fake things a
    // little bit here to make that the case. This piece-wise function is linear
    // from 0 to 64 with slope 128 and from 64 to 127 with slope 126.
    double dNorm = valueToWidgetParameter(dValue);
    return dNorm < 0.5 ? dNorm * 128.0 : dNorm * 126.0 + 1.0;
}

void ControlPotmeterBehavior::setValueFromMidiParameter(MidiOpCode o, double dParam,
                                                        ControlDoublePrivate* pControl) {
    Q_UNUSED(o);
    double dNorm = dParam < 64 ? dParam / 128.0 : (dParam - 1.0) / 126.0;
    pControl->set(widgetParameterToValue(dNorm), NULL);
}

#define maxPosition 1.0
#define minPosition 0.0
#define middlePosition ((maxPosition-minPosition)/2.0)
#define positionrange (maxPosition-minPosition)

ControlLogpotmeterBehavior::ControlLogpotmeterBehavior(double dMaxValue)
        : ControlPotmeterBehavior(0, dMaxValue) {
    if (dMaxValue == 1.0) {
        m_bTwoState = false;
        m_dB1 = log10(2.0) / maxPosition;
    } else {
        m_bTwoState = true;
        m_dB1 = log10(2.0) / middlePosition;
        m_dB2 = log10(dMaxValue) / (maxPosition - middlePosition);
    }
}

ControlLogpotmeterBehavior::~ControlLogpotmeterBehavior() {
}

double ControlLogpotmeterBehavior::defaultValue(double dDefault) const {
    Q_UNUSED(dDefault);
    return 1.0;
}

double ControlLogpotmeterBehavior::valueToWidgetParameter(double dValue) {
    if (dValue > m_dMaxValue) {
        dValue = m_dMaxValue;
    } else if (dValue < m_dMinValue) {
        dValue = m_dMinValue;
    }
    if (!m_bTwoState) {
        return log10(dValue + 1) / m_dB1;
    } else {
        if (dValue > 1.0) {
            return log10(dValue) / m_dB2 + middlePosition;
        } else {
            return log10(dValue + 1.0) / m_dB1;
        }
    }
}

double ControlLogpotmeterBehavior::widgetParameterToValue(double dParam) {
    if (!m_bTwoState) {
        return pow(10.0, m_dB1 * dParam) - 1.0;
    } else {
        if (dParam <= middlePosition) {
            return pow(10.0, m_dB1 * dParam) - 1;
        } else {
            return pow(10.0, m_dB2 * (dParam - middlePosition));
        }
    }
}

ControlLinPotmeterBehavior::ControlLinPotmeterBehavior(double dMinValue, double dMaxValue)
        : ControlPotmeterBehavior(dMinValue, dMaxValue) {
}

ControlLinPotmeterBehavior::~ControlLinPotmeterBehavior() {
}

double ControlLinPotmeterBehavior::valueToMidiParameter(double dValue) {
    // 7-bit MIDI has 128 values [0, 127]. This means there is no such thing as
    // center. The industry convention is that 64 is center. We fake things a
    // little bit here to make that the case. This function is linear from [0,
    // 127.0/128.0] with slope 128 and then cuts off at 127 from 127.0/128.0 to
    // 1.0.  from 0 to 64 with slope 128 and from 64 to 127 with slope 126.
    double dNorm = valueToWidgetParameter(dValue);
    return math_max(127.0, dNorm * 128.0);
}

void ControlLinPotmeterBehavior::setValueFromMidiParameter(MidiOpCode o, double dParam,
                                                           ControlDoublePrivate* pControl) {
    Q_UNUSED(o);
    double dNorm = dParam / 128.0;
    pControl->set(widgetParameterToValue(dNorm), NULL);
}

double ControlTTRotaryBehavior::valueToWidgetParameter(double dValue) {
    return (dValue * 200.0 + 64) / 127.0;
}

double ControlTTRotaryBehavior::widgetParameterToValue(double dParam) {
    dParam *= 127.0;
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
    } else if (m_buttonMode == TOGGLE || m_buttonMode == LONGPRESSLATCHING) {
        // This block makes push-buttons act as toggle buttons.
        if (m_iNumStates > 1) { // multistate button
            if (dParam > 0.) { // looking for NOTE_ON doesn't seem to work...
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
            } else if (o == MIDI_NOTE_OFF) {
                double value = pControl->get();
                if (m_buttonMode == LONGPRESSLATCHING &&
                        m_pushTimer.isActive() && value >= 1.0) {
                    // revert toggle if button is released too early
                    value = (int)(value - 1.0) % m_iNumStates;
                    pControl->set(value, NULL);
                }
            }
        }
    } else { // Not a toggle button (trigger only when button pushed)
        if (o == MIDI_NOTE_ON) {
            double value = pControl->get();
            pControl->set(!value, NULL);
        } else if (o == MIDI_NOTE_OFF) {
            pControl->set(0.0, NULL);
        }
    }
}
