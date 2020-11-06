#include "control/controlbehavior.h"
#include "control/control.h"
#include "util/math.h"

bool ControlNumericBehavior::setFilter(double* dValue) {
    Q_UNUSED(dValue);
    return true;
}

double ControlNumericBehavior::valueToParameter(double dValue) {
    return dValue;
}

double ControlNumericBehavior::midiToParameter(double midiValue) {
    return midiValue / 127.0;
}

double ControlNumericBehavior::parameterToValue(double dParam) {
    return dParam;
}

double ControlNumericBehavior::valueToMidiParameter(double dValue) {
    double dParam = valueToParameter(dValue);
    return dParam * 127.0;
}

void ControlNumericBehavior::setValueFromMidi(
        MidiOpCode o, double dParam, ControlDoublePrivate* pControl) {
    Q_UNUSED(o);
    double dNorm = midiToParameter(dParam);
    pControl->set(parameterToValue(dNorm), NULL);
}

double ControlEncoderBehavior::midiToParameter(double midiValue) {
    return midiValue;
}

double ControlEncoderBehavior::valueToMidiParameter(double dValue) {
    return dValue;
}

ControlPotmeterBehavior::ControlPotmeterBehavior(double dMinValue, double dMaxValue,
                                                 bool allowOutOfBounds)
        : m_dMinValue(dMinValue),
          m_dMaxValue(dMaxValue),
          m_dValueRange(m_dMaxValue - m_dMinValue),
          m_bAllowOutOfBounds(allowOutOfBounds) {
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

double ControlPotmeterBehavior::midiToParameter(double midiValue) {
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

ControlLogPotmeterBehavior::ControlLogPotmeterBehavior(double dMinValue,
        double dMaxValue, double minDB)
        : ControlPotmeterBehavior(dMinValue, dMaxValue, false) {
    if (minDB >= 0) {
        qWarning() << "ControlLogPotmeterBehavior::ControlLogPotmeterBehavior() minDB must be negative";
        m_minDB = -1;
    } else {
        m_minDB = minDB;
    }
    m_minOffset = db2ratio(m_minDB);
}

double ControlLogPotmeterBehavior::valueToParameter(double dValue) {
    if (m_dValueRange == 0.0) {
        return 0;
    }
    if (dValue > m_dMaxValue) {
        dValue = m_dMaxValue;
    } else if (dValue < m_dMinValue) {
        dValue = m_dMinValue;
    }
    double linParameter = (dValue - m_dMinValue) / m_dValueRange;
    double dbParameter = ratio2db(linParameter + m_minOffset * (1 - linParameter));
    return 1 - (dbParameter / m_minDB);
}

double ControlLogPotmeterBehavior::parameterToValue(double dParam) {
    double dbParameter = (1 - dParam) * m_minDB;
    double linParameter = (db2ratio(dbParameter) - m_minOffset) / (1 - m_minOffset);
    return m_dMinValue + (linParameter * m_dValueRange);
}

ControlLogInvPotmeterBehavior::ControlLogInvPotmeterBehavior(
        double dMinValue, double dMaxValue, double minDB)
        : ControlLogPotmeterBehavior(dMinValue, dMaxValue, minDB) {
}

double ControlLogInvPotmeterBehavior::valueToParameter(double dValue) {
    return 1 - ControlLogPotmeterBehavior::valueToParameter(dValue);
}

double ControlLogInvPotmeterBehavior::parameterToValue(double dParam) {
    return ControlLogPotmeterBehavior::parameterToValue(1 - dParam);
}

ControlLinPotmeterBehavior::ControlLinPotmeterBehavior(
        double dMinValue, double dMaxValue, bool allowOutOfBounds)
        : ControlPotmeterBehavior(dMinValue, dMaxValue, allowOutOfBounds) {
}

ControlLinInvPotmeterBehavior::ControlLinInvPotmeterBehavior(
        double dMinValue, double dMaxValue, bool allowOutOfBounds)
        : ControlPotmeterBehavior(dMinValue, dMaxValue, allowOutOfBounds) {
}

double ControlLinInvPotmeterBehavior::valueToParameter(double dValue) {
    return 1 - ControlPotmeterBehavior::valueToParameter(dValue);
}

double ControlLinInvPotmeterBehavior::parameterToValue(double dParam) {
    return ControlPotmeterBehavior::parameterToValue(1 - dParam);
}

ControlAudioTaperPotBehavior::ControlAudioTaperPotBehavior(
                             double minDB, double maxDB,
                             double neutralParameter)
        : ControlPotmeterBehavior(0.0, db2ratio(maxDB), false),
          m_neutralParameter(neutralParameter),
          m_minDB(minDB),
          m_maxDB(maxDB),
          m_offset(db2ratio(m_minDB)) {
    m_midiCorrection = ceil(m_neutralParameter * 127) - (m_neutralParameter * 127);
}

double ControlAudioTaperPotBehavior::valueToParameter(double dValue) {
    double dParam = 1.0;
    if (dValue <= 0.0) {
        return 0;
    } else if (dValue < 1.0) {
        // db + linear overlay to reach
        // m_minDB = 0
        // 0 dB = m_neutralParameter
        double overlay = m_offset * (1 - dValue);
        if (m_minDB != 0) {
            dParam = (ratio2db(dValue + overlay) - m_minDB) / m_minDB * m_neutralParameter * -1;
        } else {
            dParam = dValue * m_neutralParameter;
        }
    } else if (dValue == 1.0) {
        dParam = m_neutralParameter;
    } else if (dValue < m_dMaxValue) {
        // m_maxDB = 1
        // 0 dB = m_neutralParameter
        dParam = (ratio2db(dValue) / m_maxDB * (1 - m_neutralParameter)) + m_neutralParameter;
    }
    //qDebug() << "ControlAudioTaperPotBehavior::valueToParameter" << "value =" << dValue << "dParam =" << dParam;
    return dParam;
}

double ControlAudioTaperPotBehavior::parameterToValue(double dParam) {
    double dValue = 1;
    if (dParam <= 0.0) {
        dValue = 0;
    } else if (dParam < m_neutralParameter) {
        // db + linear overlay to reach
        // m_minDB = 0
        // 0 dB = m_neutralParameter;
        if (m_minDB != 0) {
            double db = (dParam * m_minDB / (m_neutralParameter * -1)) + m_minDB;
            dValue = (db2ratio(db) - m_offset) / (1 - m_offset) ;
        } else {
            dValue = dParam / m_neutralParameter;
        }
    } else if (dParam == m_neutralParameter) {
        dValue = 1.0;
    } else if (dParam < 1.0) {
        // m_maxDB = 1
        // 0 dB = m_neutralParame;
        dValue = db2ratio((dParam - m_neutralParameter) * m_maxDB / (1 - m_neutralParameter));
    } else {
        dValue = db2ratio(m_maxDB);
    }
    //qDebug() << "ControlAudioTaperPotBehavior::parameterToValue" << "dValue =" << dValue << "dParam =" << dParam;
    return dValue;
}

double ControlAudioTaperPotBehavior::midiToParameter(double midiValue) {
    double dParam;
    if (m_neutralParameter != 0 && m_neutralParameter != 1.0) {
        double neutralTest = (midiValue - m_midiCorrection) / 127.0;
        if (neutralTest < m_neutralParameter) {
            dParam = midiValue /
                    (127.0 + m_midiCorrection / m_neutralParameter);
        } else {
            // m_midicorrection is always < 1, so NaN check required
            dParam = (midiValue - m_midiCorrection / m_neutralParameter) /
                    (127.0 - m_midiCorrection / m_neutralParameter);
        }
    } else {
        dParam = midiValue / 127.0;
    }
    return dParam;
}

double ControlAudioTaperPotBehavior::valueToMidiParameter(double dValue) {
    // 7-bit MIDI has 128 values [0, 127]. This means there is no such thing as
    // center. The industry convention is that 64 is center.
    // We fake things a little bit here to hit the m_neutralParameter
    // always on a full Midi integer
    double dParam = valueToParameter(dValue);
    double dMidiParam = dParam * 127.0;
    if (m_neutralParameter != 0 && m_neutralParameter != 1.0) {
        if (dParam < m_neutralParameter) {
            dMidiParam += m_midiCorrection * dParam / m_neutralParameter;
        } else {
            dMidiParam += m_midiCorrection * (1 - dParam) / m_neutralParameter;
        }
    }
    return dMidiParam;
}

void ControlAudioTaperPotBehavior::setValueFromMidi(
        MidiOpCode o, double dMidiParam, ControlDoublePrivate* pControl) {
    Q_UNUSED(o);
    double dParam = midiToParameter(dMidiParam);
    pControl->set(parameterToValue(dParam), NULL);
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

void ControlPushButtonBehavior::setValueFromMidi(
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
        auto* timer = getTimer();
        if (pressed) {
            // Toggle on press
            bool value = (pControl->get() != 0);
            pControl->set(!value, nullptr);
            timer->setSingleShot(true);
            timer->start(kPowerWindowTimeMillis);
        } else if (!timer->isActive()) {
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
                    auto* timer = getTimer();
                    timer->setSingleShot(true);
                    timer->start(kLongPressLatchingTimeMillis);
                }
            } else {
                double value = pControl->get();
                if (m_buttonMode == LONGPRESSLATCHING &&
                        getTimer()->isActive() && value >= 1.) {
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
