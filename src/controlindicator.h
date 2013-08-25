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
        RATIO1TO1_500MS = 2, // used for CDJ play/pause
        RATIO1TO1_250MS = 3, // used for CDJ cue
    };

    ControlIndicator(ConfigKey key);
    virtual ~ControlIndicator();

    void setBlinkValue(enum BlinkValue bv);

  private slots:
    void slotGuiTick50ms(double streamTime);

  private:
    void toggle();

    enum BlinkValue m_blinkValue;
    bool m_on;
    double m_nextSwitchTime;
    ControlObjectThread* m_pCOTGuiTick50ms;
};

#endif // CONTROLINDICATOR_H
