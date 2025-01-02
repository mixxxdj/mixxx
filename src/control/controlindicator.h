#pragma once

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "control/pollingcontrolproxy.h"
#include "util/parented_ptr.h"

/// This type of control is used for blinking buttons such as the "play" or
/// "cue" button and makes buttons that using the same blink interval to light
/// up at the same time in the UI and on controllers.
//
/// Requires `ControlIndicatorTimer` to be initialized first.
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

    void setBlinkValue(enum BlinkValue newBlinkValue);

  signals:
    void blinkValueChanged();

  private slots:
    void slotBlinkValueChanged();
    void slotValueChanged(double value) {
        set(value);
    };

  private:
    std::atomic<BlinkValue> m_blinkValue;
    parented_ptr<ControlProxy> m_pCOIndicator250millis;
    parented_ptr<ControlProxy> m_pCOIndicator500millis;
};
