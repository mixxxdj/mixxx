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
    if (*dValue > m_dMaxValue) {
        *dValue = m_dMaxValue;
    } else if (*dValue < m_dMinValue) {
        *dValue = m_dMinValue;
    }
    return true;
}

double ControlPotmeterBehavior::defaultValue(double dDefault) const {
    Q_UNUSED(dDefault);
    return m_dDefaultValue;
}

double ControlPotmeterBehavior::valueToWidgetParameter(double dValue) {
    double dNorm = (dValue - m_dMinValue) / m_dValueRange;
    return dNorm < 0.5 ? dNorm * 128.0 : dNorm * 126.0 + 1.0;
}

double ControlPotmeterBehavior::widgetParameterToValue(double dParam) {
    double dNorm = dParam < 64 ? dParam / 128.0 : (dParam - 1.0) / 126.0;
    return m_dMinValue + dNorm * m_dValueRange;
}

double ControlPotmeterBehavior::valueToMidiParameter(double dValue) {
    return valueToWidgetParameter(dValue);
}

void ControlPotmeterBehavior::setValueFromMidiParameter(MidiOpCode o, double dParam,
                                                        ControlDoublePrivate* pControl) {
    Q_UNUSED(o);
    pControl->set(widgetParameterToValue(dParam), NULL);
}

#define maxPosition 127
#define minPosition 0
#define middlePosition ((maxPosition-minPosition)/2)
#define positionrange (maxPosition-minPosition)

ControlLogpotmeterBehavior::ControlLogpotmeterBehavior(double dMaxValue)
        : ControlPotmeterBehavior(0, dMaxValue) {
    if (dMaxValue == 1.0) {
        m_bTwoState = false;
        m_dB1 = log10(2.0)/maxPosition;
    } else {
        m_bTwoState = true;
        m_dB1 = log10(2.0) / middlePosition;
        m_dB2 = log10(dMaxValue) / (maxPosition - middlePosition);
    }
}

ControlLogpotmeterBehavior::~ControlLogpotmeterBehavior() {
}

double ControlLogpotmeterBehavior::defaultValue(double dDefault) {
    Q_UNUSED(dDefault);
    return 1.0;
}

double ControlLogpotmeterBehavior::valueToWidgetParameter(double dValue) {
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

double ControlLinPotmeterBehavior::valueToWidgetParameter(double dValue) {
    double dNorm = (dValue - m_dMinValue) / m_dValueRange;
    return math_min(dNorm * 128, 127);
}

double ControlLinPotmeterBehavior::widgetParameterToValue(double dParam) {
    double dNorm = dParam / 128.0;
    return m_dMinValue + dNorm * m_dValueRange;
}

double ControlTTRotaryBehavior::valueToWidgetParameter(double dValue) {
    return dValue * 200.0 + 64;
}

double ControlTTRotaryBehavior::widgetParameterToValue(double dParam) {
    // Non-linear scaling
    double temp = ((dParam - 64.0) * (dParam - 64.0)) / 64.0;
    if (dParam - 64 < 0) {
        temp = -temp;
    }
    return temp;
}

// static
const int ControlPushButtonBehavior::kPowerWindowTimeMillis = 300;

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

