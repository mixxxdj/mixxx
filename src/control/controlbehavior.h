#pragma once

#include <QTimer>
#include <QScopedPointer>

#include "controllers/midi/midimessage.h"

class ControlDoublePrivate;

// A linear 0 .. 1 control without Midi representation
class ControlNumericBehavior {
  public:
    virtual ~ControlNumericBehavior() { };

    // this may change the dValue in place before it is adopted
    // Returns false to reject the new value entirely
    virtual bool setFilter(double* dValue);

    // returns the normalized value range 0..1
    virtual double valueToNormalizedValue(double dValue);
    // returns the normalized value range 0..1
    virtual double midi7BitToNormalizedValue(double dMidi7BitValue);
    // returns the scaled user visible value
    virtual double normalizedValueToValue(double dNormalizedValue);
    // returns the midi range value 0..127
    virtual double valueToMidi7Bit(double dValue);

    virtual void setValueFromMidi7Bit(
            MidiOpCode o, double dNormalizedValue, ControlDoublePrivate* pControl);
};

// ControlEncoderBehavior passes the midi value directly to the internal normalized value. It's
// useful for selector knobs that pass +1 in one direction and -1 in the other.
class ControlEncoderBehavior : public ControlNumericBehavior {
  public:
    ControlEncoderBehavior() {}
    double midi7BitToNormalizedValue(double dMidi7BitValue) override;
    double valueToMidi7Bit(double dNormalizedValue) override;
};

class ControlPotmeterBehavior : public ControlNumericBehavior {
  public:
    ControlPotmeterBehavior(double dMinValue, double dMaxValue,
                            bool allowOutOfBounds);

    bool setFilter(double* dValue) override;
    double valueToNormalizedValue(double dValue) override;
    double midi7BitToNormalizedValue(double dMidi7BitValue) override;
    double normalizedValueToValue(double dNormalizedValue) override;
    double valueToMidi7Bit(double dValue) override;

  protected:
    double m_dMinValue;
    double m_dMaxValue;
    double m_dValueRange;
    bool m_bAllowOutOfBounds;
};

class ControlLogPotmeterBehavior : public ControlPotmeterBehavior {
  public:
    ControlLogPotmeterBehavior(double dMinValue, double dMaxValue, double minDB);

    double valueToNormalizedValue(double dValue) override;
    double normalizedValueToValue(double dNormalizedValue) override;

  protected:
    double m_minDB;
    double m_minOffset;
};

class ControlLogInvPotmeterBehavior : public ControlLogPotmeterBehavior {
  public:
    ControlLogInvPotmeterBehavior(double dMinValue, double dMaxValue, double minDB);

    double valueToNormalizedValue(double dValue) override;
    double normalizedValueToValue(double dNormalizedValue) override;
};

class ControlLinPotmeterBehavior : public ControlPotmeterBehavior {
  public:
    ControlLinPotmeterBehavior(
            double dMinValue, double dMaxValue, bool allowOutOfBounds);
};

class ControlLinInvPotmeterBehavior : public ControlPotmeterBehavior {
  public:
    ControlLinInvPotmeterBehavior(
            double dMinValue, double dMaxValue, bool allowOutOfBounds);
    double valueToNormalizedValue(double dValue) override;
    double normalizedValueToValue(double dNormalizedValue) override;
};

class ControlAudioTaperPotBehavior : public ControlPotmeterBehavior {
  public:
    ControlAudioTaperPotBehavior(double minDB, double maxDB, double neutralNormalizedValue);

    double valueToNormalizedValue(double dValue) override;
    double normalizedValueToValue(double dNormalizedValue) override;
    double midi7BitToNormalizedValue(double dMidi7BitValue) override;
    double valueToMidi7Bit(double dValue) override;
    void setValueFromMidi7Bit(
            MidiOpCode o, double dNormalizedValue, ControlDoublePrivate* pControl)
            override;

  protected:
    // a knob position between 0 and 1 where the gain is 1 (0dB)
    double m_neutralNormalizedValue;
    // the Start value of the pure db scale it cranked to -Infinity by the
    // linear part of the AudioTaperPot
    double m_minDB;
    // maxDB is the upper gain Value
    double m_maxDB;
    // offset at knob position 0 (normalized value = 0) to reach -Infinity
    double m_offset;
    // ensures that the neutral position on a integer midi value
    // This value is subtracted from the Midi value at neutral position
    // and is always < 1
    double m_midiCorrection;
};

class ControlTTRotaryBehavior : public ControlNumericBehavior {
  public:
    double valueToNormalizedValue(double dValue) override;
    double normalizedValueToValue(double dNormalizedValue) override;
};

class ControlPushButtonBehavior : public ControlNumericBehavior {
  public:
    static const int kPowerWindowTimeMillis;
    static const int kLongPressLatchingTimeMillis;

    // TODO(XXX) Duplicated from ControlPushButton. It's complicated and
    // annoying to share them so I just copied them.
    enum ButtonMode {
         PUSH = 0,
         TOGGLE,
         POWERWINDOW,
         LONGPRESSLATCHING,
         TRIGGER
    };

    ControlPushButtonBehavior(ButtonMode buttonMode, int iNumStates);
    void setValueFromMidi7Bit(
            MidiOpCode o, double dNormalizedValue, ControlDoublePrivate* pControl)
            override;

  private:
    // We create many hundreds of push buttons at Mixxx startup and most of them
    // never use their timer. Delay creation of the timer until it's needed.
    QTimer* getTimer() {
        if (m_pushTimer.isNull()) {
            m_pushTimer.reset(new QTimer());
        }
        return m_pushTimer.data();
    }
    ButtonMode m_buttonMode;
    int m_iNumStates;
    QScopedPointer<QTimer> m_pushTimer;
};

class ControlLinSteppedIntPotBehavior : public ControlPotmeterBehavior {
  public:
    ControlLinSteppedIntPotBehavior(
            double dMinValue, double dMaxValue, bool allowOutOfBounds);

    double valueToNormalizedValue(double dValue) override;
    double normalizedValueToValue(double dNormalizedValue) override;

  protected:
    double m_lastSnappedNormalizedValue;
    double m_dist;
    double m_oldVal;
};
