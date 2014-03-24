#ifndef CONTROLBEHAVIOR_H
#define CONTROLBEHAVIOR_H

#include <math.h>

#include <QTimer>

#include "controllers/midi/midimessage.h"
#include "mathstuff.h"

class ControlDoublePrivate;

class ControlNumericBehavior {
  public:
    virtual ~ControlNumericBehavior() { };

    // Returns true if the set should occur. Mutates dValue if the value should
    // be changed.
    virtual bool setFilter(double* dValue);

    virtual double defaultValue(double dDefault) const;
    virtual double valueToParameter(double dValue);
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
    virtual double parameterToValue(double dParam);
    virtual double valueToMidiParameter(double dValue);
    virtual void setValueFromMidiParameter(MidiOpCode o, double dParam,
                                           ControlDoublePrivate* pControl);

  protected:
    double m_dMinValue;
    double m_dMaxValue;
    double m_dValueRange;
    double m_dDefaultValue;
    bool m_bAllowOutOfBounds;
};

class ControlLogpotmeterBehavior : public ControlPotmeterBehavior {
  public:
    ControlLogpotmeterBehavior(double dMaxValue);
    virtual ~ControlLogpotmeterBehavior();

    virtual double defaultValue(double dDefault) const;
    virtual double valueToParameter(double dValue);
    virtual double parameterToValue(double dParam);

  protected:
    bool m_bTwoState;
    double m_dB1, m_dB2;
};

class ControlLinPotmeterBehavior : public ControlPotmeterBehavior {
  public:
    ControlLinPotmeterBehavior(double dMinValue, double dMaxValue);
    virtual ~ControlLinPotmeterBehavior();

    virtual double valueToMidiParameter(double dValue);
    virtual void setValueFromMidiParameter(MidiOpCode o, double dParam,
                                           ControlDoublePrivate* pControl);
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
