#include "control/controlbehavior.h"
#include "control/control.h"
#include "util/math.h"

bool ControlNumericBehavior::setFilter(double* dValue) {
    Q_UNUSED(dValue);
    return true;
}

double ControlNumericBehavior::valueToNormalizedValue(double dValue) {
    // TODO: This does not scale - Do we need a warning if this is called?
    return dValue;
}

double ControlNumericBehavior::midi7BitToNormalizedValue(double midiValue) {
    return midiValue / 127.0;
}

double ControlNumericBehavior::normalizedValueToValue(double dNormalizedValue) {
    // TODO: This does not scale - Do we need a warning if this is called?
    return dNormalizedValue;
}

double ControlNumericBehavior::valueToMidi7Bit(double dValue) {
    double dNormalizedValue = valueToNormalizedValue(dValue);
    return dNormalizedValue * 127.0;
}

void ControlNumericBehavior::setValueFromMidi7Bit(
        MidiOpCode o, double dNormalizedValue, ControlDoublePrivate* pControl) {
    Q_UNUSED(o);
    double dNorm = midi7BitToNormalizedValue(dNormalizedValue);
    pControl->set(normalizedValueToValue(dNorm), nullptr);
}

double ControlEncoderBehavior::midi7BitToNormalizedValue(double midiValue) {
    // TODO: This does not scale - Do we need a warning if this is called?
    return midiValue;
}

double ControlEncoderBehavior::valueToMidi7Bit(double dValue) {
    // TODO: This does not scale - Do we need a warning if this is called?
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

double ControlPotmeterBehavior::valueToNormalizedValue(double dValue) {
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

double ControlPotmeterBehavior::midi7BitToNormalizedValue(double midiValue) {
    double normalizedValue;
    if (midiValue > 64) {
        normalizedValue = (midiValue - 1) / 126.0;
    } else {
        // Hack for 0.5 at 64
        normalizedValue = midiValue / 128.0;
    }
    return normalizedValue;
}

double ControlPotmeterBehavior::normalizedValueToValue(double dNormalizedValue) {
    return m_dMinValue + (dNormalizedValue * m_dValueRange);
}

double ControlPotmeterBehavior::valueToMidi7Bit(double dValue) {
    // 7-bit MIDI has 128 values [0, 127]. This means there is no such thing as
    // center. The industry convention is that 64 is center. We fake things a
    // little bit here to make that the case. This function is linear from [0,
    // 127.0/128.0] with slope 128 and then cuts off at 127 from 127.0/128.0 to
    // 1.0.  from 0 to 64 with slope 128 and from 64 to 127 with slope 126.
    double dNorm = valueToNormalizedValue(dValue);
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

double ControlLogPotmeterBehavior::valueToNormalizedValue(double dValue) {
    if (m_dValueRange == 0.0) {
        return 0;
    }
    if (dValue > m_dMaxValue) {
        dValue = m_dMaxValue;
    } else if (dValue < m_dMinValue) {
        dValue = m_dMinValue;
    }
    double dNormalizedLinValue = (dValue - m_dMinValue) / m_dValueRange;
    double dNormalizedDbValue = ratio2db(
            dNormalizedLinValue + m_minOffset * (1 - dNormalizedLinValue));
    return 1 - (dNormalizedDbValue / m_minDB);
}

double ControlLogPotmeterBehavior::normalizedValueToValue(double dNormalizedValue) {
    double dNormalizedDbValue = (1 - dNormalizedValue) * m_minDB;
    double dNormalizedLinValue = (db2ratio(dNormalizedDbValue) - m_minOffset) / (1 - m_minOffset);
    return m_dMinValue + (dNormalizedLinValue * m_dValueRange);
}

ControlLogInvPotmeterBehavior::ControlLogInvPotmeterBehavior(
        double dMinValue, double dMaxValue, double minDB)
        : ControlLogPotmeterBehavior(dMinValue, dMaxValue, minDB) {
}

double ControlLogInvPotmeterBehavior::valueToNormalizedValue(double dNormalizedValue) {
    return 1 - ControlLogPotmeterBehavior::valueToNormalizedValue(dNormalizedValue);
}

double ControlLogInvPotmeterBehavior::normalizedValueToValue(double dNormalizedValue) {
    return ControlLogPotmeterBehavior::normalizedValueToValue(1 - dNormalizedValue);
}

ControlLinPotmeterBehavior::ControlLinPotmeterBehavior(
        double dMinValue, double dMaxValue, bool allowOutOfBounds)
        : ControlPotmeterBehavior(dMinValue, dMaxValue, allowOutOfBounds) {
}

ControlLinInvPotmeterBehavior::ControlLinInvPotmeterBehavior(
        double dMinValue, double dMaxValue, bool allowOutOfBounds)
        : ControlPotmeterBehavior(dMinValue, dMaxValue, allowOutOfBounds) {
}

double ControlLinInvPotmeterBehavior::valueToNormalizedValue(double dValue) {
    return 1 - ControlPotmeterBehavior::valueToNormalizedValue(dValue);
}

double ControlLinInvPotmeterBehavior::normalizedValueToValue(double dNormalizedValue) {
    return ControlPotmeterBehavior::normalizedValueToValue(1 - dNormalizedValue);
}

ControlAudioTaperPotBehavior::ControlAudioTaperPotBehavior(
        double minDB, double maxDB, double neutralNormalizedValue)
        : ControlPotmeterBehavior(0.0, db2ratio(maxDB), false),
          m_neutralNormalizedValue(neutralNormalizedValue),
          m_minDB(minDB),
          m_maxDB(maxDB),
          m_offset(db2ratio(m_minDB)) {
    m_midiCorrection = ceil(m_neutralNormalizedValue * 127) - (m_neutralNormalizedValue * 127);
}

double ControlAudioTaperPotBehavior::valueToNormalizedValue(double dValue) {
    double dNormalizedValue = 1.0;
    if (dValue <= 0.0) {
        return 0;
    } else if (dValue < 1.0) {
        // db + linear overlay to reach
        // m_minDB = 0
        // 0 dB = m_neutralNormalizedValue
        double overlay = m_offset * (1 - dValue);
        if (m_minDB != 0) {
            dNormalizedValue = (ratio2db(dValue + overlay) - m_minDB) /
                    m_minDB * m_neutralNormalizedValue * -1;
        } else {
            dNormalizedValue = dValue * m_neutralNormalizedValue;
        }
    } else if (dValue == 1.0) {
        dNormalizedValue = m_neutralNormalizedValue;
    } else if (dValue < m_dMaxValue) {
        // m_maxDB = 1
        // 0 dB = m_neutralNormalizedValue
        dNormalizedValue =
                (ratio2db(dValue) / m_maxDB * (1 - m_neutralNormalizedValue)) +
                m_neutralNormalizedValue;
    }
    // qDebug() << "ControlAudioTaperPotBehavior::valueToNormalizedValue" <<
    // "value =" << dNormalizedValue << "dNormalizedValue =" << dNormalizedValue;
    return dNormalizedValue;
}

double ControlAudioTaperPotBehavior::normalizedValueToValue(double dNormalizedValue) {
    double dValue = 1;
    if (dNormalizedValue <= 0.0) {
        dValue = 0;
    } else if (dNormalizedValue < m_neutralNormalizedValue) {
        // db + linear overlay to reach
        // m_minDB = 0
        // 0 dB = m_neutralNormalizedValue;
        if (m_minDB != 0) {
            double db = (dNormalizedValue * m_minDB / (m_neutralNormalizedValue * -1)) + m_minDB;
            dValue = (db2ratio(db) - m_offset) / (1 - m_offset) ;
        } else {
            dValue = dNormalizedValue / m_neutralNormalizedValue;
        }
    } else if (dNormalizedValue == m_neutralNormalizedValue) {
        dValue = 1.0;
    } else if (dNormalizedValue < 1.0) {
        // m_maxDB = 1
        // 0 dB = m_neutralParame;
        dValue = db2ratio((dNormalizedValue - m_neutralNormalizedValue) *
                m_maxDB / (1 - m_neutralNormalizedValue));
    } else {
        dValue = db2ratio(m_maxDB);
    }
    // qDebug() << "ControlAudioTaperPotBehavior::normalizedValueToValue" <<
    // "dNormalizedValue =" << dNormalizedValue << "dNormalizedValue =" << dNormalizedValue;
    return dValue;
}

double ControlAudioTaperPotBehavior::midi7BitToNormalizedValue(double midiValue) {
    double dNormalizedValue;
    if (m_neutralNormalizedValue != 0 && m_neutralNormalizedValue != 1.0) {
        double neutralTest = (midiValue - m_midiCorrection) / 127.0;
        if (neutralTest < m_neutralNormalizedValue) {
            dNormalizedValue = midiValue /
                    (127.0 + m_midiCorrection / m_neutralNormalizedValue);
        } else {
            // m_midicorrection is always < 1, so NaN check required
            dNormalizedValue = (midiValue - m_midiCorrection / m_neutralNormalizedValue) /
                    (127.0 - m_midiCorrection / m_neutralNormalizedValue);
        }
    } else {
        dNormalizedValue = midiValue / 127.0;
    }
    return dNormalizedValue;
}

double ControlAudioTaperPotBehavior::valueToMidi7Bit(double dValue) {
    // 7-bit MIDI has 128 values [0, 127]. This means there is no such thing as
    // center. The industry convention is that 64 is center.
    // We fake things a little bit here to hit the m_neutralNormalizedValue
    // always on a full Midi integer
    double dNormalizedValue = valueToNormalizedValue(dValue);
    double dMidiParam = dNormalizedValue * 127.0;
    if (m_neutralNormalizedValue != 0 && m_neutralNormalizedValue != 1.0) {
        if (dNormalizedValue < m_neutralNormalizedValue) {
            dMidiParam += m_midiCorrection * dNormalizedValue / m_neutralNormalizedValue;
        } else {
            dMidiParam += m_midiCorrection * (1 - dNormalizedValue) / m_neutralNormalizedValue;
        }
    }
    return dMidiParam;
}

void ControlAudioTaperPotBehavior::setValueFromMidi7Bit(
        MidiOpCode o, double dMidiParam, ControlDoublePrivate* pControl) {
    Q_UNUSED(o);
    double dNormalizedValue = midi7BitToNormalizedValue(dMidiParam);
    pControl->set(normalizedValueToValue(dNormalizedValue), nullptr);
}

double ControlTTRotaryBehavior::valueToNormalizedValue(double dValue) {
    return (dValue * 200.0 + 64) / 127.0;
}

double ControlTTRotaryBehavior::normalizedValueToValue(double dNormalizedValue) {
    dNormalizedValue *= 128.0;
    // Non-linear scaling
    double temp = ((dNormalizedValue - 64.0) * (dNormalizedValue - 64.0)) / 64.0;
    if (dNormalizedValue - 64 < 0) {
        temp = -temp;
    }
    return temp;
}

ControlLinSteppedIntPotBehavior::ControlLinSteppedIntPotBehavior(
        double dMinValue, double dMaxValue, bool allowOutOfBounds)
        : ControlPotmeterBehavior(dMinValue, dMaxValue, false) {
    m_dMinValue = round(dMinValue);
    m_dMaxValue = round(dMaxValue);
    m_dValueRange = m_dMaxValue - m_dMinValue;
    m_bAllowOutOfBounds = allowOutOfBounds;
    m_lastSnappedNormalizedValue = 0;
    m_dist = 0;
    m_oldVal = 0;
}

double ControlLinSteppedIntPotBehavior::valueToNormalizedValue(double dValue) {
    if (m_dValueRange == 0.0) {
        return 0;
    }
    if (dValue > m_dMaxValue) {
        dValue = m_dMaxValue;
    } else if (dValue < m_dMinValue) {
        dValue = m_dMinValue;
    }
    double param = (dValue - m_dMinValue) / m_dValueRange;
    return param;
}

double ControlLinSteppedIntPotBehavior::normalizedValueToValue(double dNormalizedValue) {
    // Note: each value change will make ControlDoublePrivate::setInner emit
    // valueChanged(), call valueToNormalizedValue() and call
    // normalizedValueToValue() again with snapped normalized value. Thus we
    // can't compare dNormalizedValue to the previous because that would simply
    // swap the sign of resulting dist with each cal. Instead, compare to
    // m_lastSnappedNormalizedValue and accumulate all change request deltas.
    if (dNormalizedValue == m_lastSnappedNormalizedValue) {
        return round(m_dMinValue + (dNormalizedValue * m_dValueRange));
    }

    double dist = dNormalizedValue - m_lastSnappedNormalizedValue;
    // compare current and previous change direction
    if (m_dist != 0 && (dist > 0) == (m_dist > 0)) { // same direction > add
        m_dist += dist;
    } else { // new direction or m_dist is 0 > replace
        m_dist = dist;
    }

    double newVal = round(m_dMinValue + ((m_lastSnappedNormalizedValue + m_dist) * m_dValueRange));
    if (newVal != m_oldVal) {
        m_lastSnappedNormalizedValue = valueToNormalizedValue(newVal);
        m_oldVal = newVal;
        m_dist = 0;
    }
    return newVal;
}

// static
const int ControlPushButtonBehavior::kPowerWindowTimeMillis = 300;
const int ControlPushButtonBehavior::kLongPressLatchingTimeMillis = 300;

ControlPushButtonBehavior::ControlPushButtonBehavior(ButtonMode buttonMode,
                                                     int iNumStates)
        : m_buttonMode(buttonMode),
          m_iNumStates(iNumStates) {
}

void ControlPushButtonBehavior::setValueFromMidi7Bit(
        MidiOpCode o, double dNormalizedValue, ControlDoublePrivate* pControl) {
    // Calculate pressed State of the midi Button
    // Some controller like the RMX2 are sending always MidiOpCode::NoteOn
    // with a changed dNormalizedValue 127 for pressed an 0 for released.
    // Other controller like the VMS4 are using MidiOpCode::NoteOn
    // And MidiOpCode::NoteOff and a velocity value like a piano keyboard
    bool pressed = true;
    if (o == MidiOpCode::NoteOff || dNormalizedValue == 0) {
        // MidiOpCode::NoteOn + 0 should be interpreted a released according to
        // http://de.wikipedia.org/wiki/Musical_Instrument_Digital_Interface
        // looking for MidiOpCode::NoteOn doesn't seem to work...
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
            pControl->set(0., nullptr);
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
                pControl->set(value, nullptr);
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
                    pControl->set(value, nullptr);
                }
            }
        }
    } else { // Not a toggle button (trigger only when button pushed)
        if (pressed) {
            pControl->set(1., nullptr);
        } else {
            pControl->set(0., nullptr);
        }
    }
}
