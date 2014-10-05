#ifndef CONTROLBEHAVIOR_H
#define CONTROLBEHAVIOR_H

#include <QTimer>

#include "controllers/midi/midimessage.h"

class ControlDoublePrivate;

class ControlNumericBehavior {
  public:
    virtual ~ControlNumericBehavior() { };

    // Returns true if the set should occur. Mutates dValue if the value should
    // be changed.
    virtual bool setFilter(double* dValue);

    virtual double valueToParameter(double dValue);
    virtual double midiValueToParameter(double midiValue);
    virtual double parameterToValue(double dParam);
    virtual double valueToMidiParameter(double dValue);
    virtual void setValueFromMidiParameter(MidiOpCode o, double dParam,
                                           ControlDoublePrivate* pControl);
};

class ControlPotmeterBehavior : public ControlNumericBehavior {
  public:
    ControlPotmeterBehavior(double dMinValue, double dMaxValue,
                            bool allowOutOfBounds);
    virtual ~ControlPotmeterBehavior();

    virtual bool setFilter(double* dValue);
    virtual double valueToParameter(double dValue);
    virtual double midiValueToParameter(double midiValue);
    virtual double parameterToValue(double dParam);
    virtual double valueToMidiParameter(double dValue);

  protected:
    double m_dMinValue;
    double m_dMaxValue;
    double m_dValueRange;
    bool m_bAllowOutOfBounds;
};

class ControlLogPotmeterBehavior : public ControlPotmeterBehavior {
  public:
    ControlLogPotmeterBehavior(double dMinValue, double dMaxValue, double minDB);
    virtual ~ControlLogPotmeterBehavior();

    virtual double valueToParameter(double dValue);
    virtual double parameterToValue(double dParam);

  protected:
    double m_minDB;
    double m_minOffset;
};

class ControlLinPotmeterBehavior : public ControlPotmeterBehavior {
  public:
    ControlLinPotmeterBehavior(double dMinValue, double dMaxValue,
                               bool allowOutOfBounds);
    virtual ~ControlLinPotmeterBehavior();
};

class ControlAudioTaperPotBehavior : public ControlPotmeterBehavior {
  public:
    ControlAudioTaperPotBehavior(double minDB, double maxDB,
                                 double neutralParameter);
    virtual ~ControlAudioTaperPotBehavior();

    virtual double valueToParameter(double dValue);
    virtual double parameterToValue(double dParam);
    virtual double midiValueToParameter(double midiValue);
    virtual double valueToMidiParameter(double dValue);
    virtual void setValueFromMidiParameter(MidiOpCode o, double dParam,
                                           ControlDoublePrivate* pControl);

  protected:
    // a knob position between 0 and 1 where the gain is 1 (0dB)
    double m_neutralParameter;
    // the Start value of the pure db scale it cranked to -Infinity by the
    // linear part of the AudioTaperPot
    double m_minDB;
    // maxDB is the upper gain Value
    double m_maxDB;
    // offset at knob position 0 (Parameter = 0) to reach -Infinity
    double m_offset;
    // ensures that the neutral position on a integer midi value
    // This value is subtracted from the Midi value at neutral position
    // and is allways < 1
    double m_midiCorrection;
};

class ControlTTRotaryBehavior : public ControlNumericBehavior {
  public:
    virtual double valueToParameter(double dValue);
    virtual double parameterToValue(double dParam);
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
    };

    ControlPushButtonBehavior(ButtonMode buttonMode, int iNumStates);
    virtual void setValueFromMidiParameter(MidiOpCode o, double dParam,
                                           ControlDoublePrivate* pControl);

  private:
    ButtonMode m_buttonMode;
    int m_iNumStates;
    QTimer m_pushTimer;
};

#endif /* CONTROLBEHAVIOR_H */
