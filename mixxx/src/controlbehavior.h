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
    virtual bool setFilter(double* dValue) {
        Q_UNUSED(dValue);
        return true;
    }

    virtual double defaultValue(double dDefault) const {
        return dDefault;
    }

    virtual double valueToWidgetParameter(double dValue) {
        return dValue;
    }

    virtual double widgetParameterToValue(double dParam) {
        return dParam;
    }

    virtual double valueToMidiParameter(double dValue) {
        return dValue;
    }

    virtual void setValueFromMidiParameter(MidiOpCode o, double dParam,
                                           ControlDoublePrivate* pControl);
};

class ControlPotmeterBehavior : public ControlNumericBehavior {
  public:
    ControlPotmeterBehavior(double dMinValue, double dMaxValue)
            : m_dMinValue(dMinValue),
              m_dMaxValue(dMaxValue),
              m_dValueRange(m_dMaxValue - m_dMinValue),
              m_dDefaultValue(m_dMinValue + 0.5 * m_dValueRange) {
    }

    virtual bool setFilter(double* dValue) {
        if (*dValue > m_dMaxValue) {
            *dValue = m_dMaxValue;
        } else if (*dValue < m_dMinValue) {
            *dValue = m_dMinValue;
        }
        return true;
    }

    virtual double defaultValue(double dDefault) const {
        Q_UNUSED(dDefault);
        return m_dDefaultValue;
    }

    virtual double valueToWidgetParameter(double dValue) {
        double dNorm = (dValue - m_dMinValue) / m_dValueRange;
        return dNorm < 0.5 ? dNorm * 128.0 : dNorm * 126.0 + 1.0;
    }

    virtual double widgetParameterToValue(double dParam) {
        double dNorm = dParam < 64 ? dParam / 128.0 : (dParam - 1.0) / 126.0;
        return m_dMinValue + dNorm * m_dValueRange;
    }

    virtual double valueToMidiParameter(double dValue) {
        return valueToWidgetParameter(dValue);
    }

    virtual void setValueFromMidiParameter(MidiOpCode o, double dParam,
                                           ControlDoublePrivate* pControl);

  protected:
    double m_dMinValue;
    double m_dMaxValue;
    double m_dValueRange;
    double m_dDefaultValue;
};

#define maxPosition 127
#define minPosition 0
#define middlePosition ((maxPosition-minPosition)/2)
#define positionrange (maxPosition-minPosition)

class ControlLogpotmeterBehavior : public ControlPotmeterBehavior {
  public:
    ControlLogpotmeterBehavior(double dMaxValue) : ControlPotmeterBehavior(0, dMaxValue) {
        m_dB1 = log10(2.0) / middlePosition;
        m_dB2 = log10(dMaxValue) / (maxPosition - middlePosition);
    }

    virtual double defaultValue(double dDefault) {
        Q_UNUSED(dDefault);
        return 1.0;
    }

    virtual double valueToWidgetParameter(double dValue) {
        if (dValue > 1.0) {
            return log10(dValue) / m_dB2 + middlePosition;
        } else {
            return log10(dValue + 1.0) / m_dB1;
        }
    }

    virtual double widgetParameterToValue(double dParam) {
        if (dParam <= middlePosition) {
            return pow(10.0, m_dB1 * dParam) - 1;
        } else {
            return pow(10.0, m_dB2 * (dParam - middlePosition));
        }
    }

  protected:
    double m_dB1, m_dB2;
};

class ControlLinPotmeterBehavior : public ControlPotmeterBehavior {
  public:
    ControlLinPotmeterBehavior(double dMinValue, double dMaxValue)
            : ControlPotmeterBehavior(dMinValue, dMaxValue) {
    }

    virtual double valueToWidgetParameter(double dValue) {
        double dNorm = (dValue - m_dMinValue) / m_dValueRange;
        return math_min(dNorm * 128, 127);
    }

    virtual double widgetParameterToValue(double dParam) {
        double dNorm = dParam / 128.0;
        return m_dMinValue + dNorm * m_dValueRange;
    }
};

class ControlTTRotaryBehavior : public ControlNumericBehavior {
  public:
    virtual double valueToWidgetParameter(double dValue) {
        return dValue * 200.0 + 64;
    }

    virtual double widgetParameterToValue(double dParam) {
        // Non-linear scaling
        double temp = ((dParam - 64.0) * (dParam - 64.0)) / 64.0;
        if (dParam - 64 < 0) {
            temp = -temp;
        }
        return temp;
    }
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

    ControlPushButtonBehavior(ButtonMode buttonMode,
                              int iNumStates)
            : m_buttonMode(buttonMode),
              m_iNumStates(iNumStates) {
    }

    virtual void setValueFromMidiParameter(MidiOpCode o, double dParam,
                                           ControlDoublePrivate* pControl);

  private:
    ButtonMode m_buttonMode;
    int m_iNumStates;
    QTimer m_pushTimer;
};

#endif /* CONTROLBEHAVIOR_H */
