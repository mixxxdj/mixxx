#ifndef CONTROLINDICATOR_H
#define CONTROLINDICATOR_H

#include "controlobject.h"

class ControlObjectThread;

class ControlIndicator : public ControlObject {
    Q_OBJECT
  public:
    enum BlinkValue {
        OFF = 0,
        ON = 1,
        RATIO1TO1_500MS = 2, // used for Pioneer play/pause
        RATIO1TO1_250MS = 3, // used for Pioneer cue
    };

    ControlIndicator(ConfigKey key);
    virtual ~ControlIndicator();

    void setBlinkValue(enum BlinkValue bv);

  private slots:
    void slotGuiTick50ms(double cpuTime);

  private:
    void toggle();
    // set() is private, use setBlinkValue instead
    void set(double value) { ControlObject::set(value); };

    enum BlinkValue m_blinkValue;
    double m_nextSwitchTime;
    ControlObjectThread* m_pCOTGuiTickTime;
    ControlObjectThread* m_pCOTGuiTick50ms;
};

#endif // CONTROLINDICATOR_H
