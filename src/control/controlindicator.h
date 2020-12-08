#pragma once

#include "control/controlobject.h"

class ControlProxy;

class ControlIndicator : public ControlObject {
    Q_OBJECT
  public:
    enum BlinkValue {
        OFF = 0,
        ON = 1,
        RATIO1TO1_500MS = 2, // used for Pioneer play/pause
        RATIO1TO1_250MS = 3, // used for Pioneer cue
    };

    ControlIndicator(const ConfigKey& key);
    virtual ~ControlIndicator();

    void setBlinkValue(enum BlinkValue bv);

  signals:
    void blinkValueChanged();

  private slots:
    void slotGuiTick50ms(double cpuTime);
    void slotBlinkValueChanged();

  private:
    void toggle(double duration);
    // set() is private, use setBlinkValue instead
    // it must be called from the GUI thread only to a void
    // race condition by toggle()
    void set(double value) { ControlObject::set(value); };

    enum BlinkValue m_blinkValue;
    double m_nextSwitchTime;
    ControlProxy* m_pCOTGuiTickTime;
    ControlProxy* m_pCOTGuiTick50ms;
};
