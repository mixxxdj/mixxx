#pragma once

#include "control/controlobject.h"

class ControlPushButton : public ControlObject {
    Q_OBJECT
  public:
    enum ButtonMode {
        /// Default mode - the value is set to 1 on press and 0 on release
        PUSH = 0,
        /// Each press increments the value by one, wrapping back to 0 when the number of states is exceeded
        TOGGLE,
        /// Behaves like TOGGLE if released quickly, otherwise PUSH
        POWERWINDOW,
        /// Toggle if the button is held for a specific amount of time
        LONGPRESSLATCHING,
        /// Only emit on press
        TRIGGER,
    };

    static QString buttonModeToString(int mode) {
        switch(mode) {
            case ControlPushButton::PUSH:
                return "PUSH";
            case ControlPushButton::TOGGLE:
                return "TOGGLE";
            case ControlPushButton::POWERWINDOW:
                return "POWERWINDOW";
            case ControlPushButton::LONGPRESSLATCHING:
                return "LONGPRESSLATCHING";
            case ControlPushButton::TRIGGER:
                return "TRIGGER";
            default:
                return "UNKNOWN";
        }
    }

    ControlPushButton(const ConfigKey& key, bool bPersist = false, double defaultValue = 0.0);
    virtual ~ControlPushButton();

    inline ButtonMode getButtonMode() const {
        return m_buttonMode;
    }
    void setButtonMode(enum ButtonMode mode);
    void setStates(int num_states);

  private:
    enum ButtonMode m_buttonMode;
    int m_iNoStates;
};
