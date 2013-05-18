#ifndef CONTROLBEHAVIOR_H
#define CONTROLBEHAVIOR_H

#include <math.h>

#include <QTimer>

#include "controllers/midi/midimessage.h"
#include "mathstuff.h"

class ControlDoublePrivate;

class ControlNumericBehavior {
  public:
    // Returns true if the set should occur. Mutates dValue if the value should
    // be changed.
    virtual bool setFilter(double* dValue);

    virtual double defaultValue(double dDefault) const;
    virtual double valueToWidgetParameter(double dValue);
    virtual double widgetParameterToValue(double dParam);
    virtual double valueToMidiParameter(double dValue);
    virtual void setValueFromMidiParameter(MidiOpCode o, double dParam,
                                           ControlDoublePrivate* pControl);
};

class ControlPotmeterBehavior : public ControlNumericBehavior {
  public:
    ControlPotmeterBehavior(double dMinValue, double dMaxValue);
    virtual ~ControlPotmeterBehavior();

    virtual bool setFilter(double* dValue);
    virtual double defaultValue(double dDefault) const;
    virtual double valueToWidgetParameter(double dValue);
    virtual double widgetParameterToValue(double dParam);
    virtual double valueToMidiParameter(double dValue);
    virtual void setValueFromMidiParameter(MidiOpCode o, double dParam,
                                           ControlDoublePrivate* pControl);

  protected:
    double m_dMinValue;
    double m_dMaxValue;
    double m_dValueRange;
    double m_dDefaultValue;
};

class ControlLogpotmeterBehavior : public ControlPotmeterBehavior {
  public:
    ControlLogpotmeterBehavior(double dMaxValue);
    virtual ~ControlLogpotmeterBehavior();

    virtual double defaultValue(double dDefault);
    virtual double valueToWidgetParameter(double dValue);
    virtual double widgetParameterToValue(double dParam);

  protected:
    double m_dB1, m_dB2;
};

class ControlLinPotmeterBehavior : public ControlPotmeterBehavior {
  public:
    ControlLinPotmeterBehavior(double dMinValue, double dMaxValue);
    virtual ~ControlLinPotmeterBehavior();

    virtual double valueToWidgetParameter(double dValue);
    virtual double widgetParameterToValue(double dParam);
};

class ControlTTRotaryBehavior : public ControlNumericBehavior {
  public:
    virtual double valueToWidgetParameter(double dValue);
    virtual double widgetParameterToValue(double dParam);
};

class ControlPushButtonBehavior : public ControlNumericBehavior {
  public:
    static const int kPowerWindowTimeMillis;

    // TODO(XXX) Duplicated from ControlPushButton. It's complicated and
    // annoying to share them so I just copied them.
    enum ButtonMode {
         PUSH = 0,
         TOGGLE,
         POWERWINDOW
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
