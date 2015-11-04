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

    virtual double defaultValue(double dDefault) const;
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
    virtual double defaultValue(double dDefault) const;
    virtual double valueToParameter(double dValue);
    virtual double midiValueToParameter(double midiValue);
    virtual double parameterToValue(double dParam);
    virtual double valueToMidiParameter(double dValue);

  protected:
    double m_dMinValue;
    double m_dMaxValue;
    double m_dValueRange;
    double m_dDefaultValue;
    bool m_bAllowOutOfBounds;
};

class ControlLogPotmeterBehavior : public ControlPotmeterBehavior {
  public:
    ControlLogPotmeterBehavior(double dMinValue, double dMaxValue);
    virtual ~ControlLogPotmeterBehavior();

    virtual double defaultValue(double dDefault) const;
    virtual double valueToParameter(double dValue);
    virtual double parameterToValue(double dParam);

  protected:
    bool m_bTwoState;
    double m_dB1, m_dB2;
};

class ControlLinPotmeterBehavior : public ControlPotmeterBehavior {
  public:
    ControlLinPotmeterBehavior(double dMinValue, double dMaxValue,
                               bool allowOutOfBounds);
    virtual ~ControlLinPotmeterBehavior();
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
