#pragma once

#include "control/controlbuttonmode.h"
#include "control/controlobject.h"

class ControlPushButton : public ControlObject {
    Q_OBJECT
  public:
    static QString buttonModeToString(ControlButtonMode mode) {
        switch(mode) {
        case ControlButtonMode::Push:
            return QStringLiteral("Push");
        case ControlButtonMode::Toggle:
            return QStringLiteral("Toggle");
        case ControlButtonMode::PowerWindow:
            return QStringLiteral("PowerWindow");
        case ControlButtonMode::LongPressLatching:
            return QStringLiteral("LongPressLatching");
        case ControlButtonMode::Trigger:
            return QStringLiteral("Trigger");
        }
        DEBUG_ASSERT(false);
        return "Unknown";
    }

    ControlPushButton(const ConfigKey& key, bool bPersist = false, double defaultValue = 0.0);
    virtual ~ControlPushButton();

    inline ControlButtonMode getButtonMode() const {
        return m_buttonMode;
    }
    void setButtonMode(ControlButtonMode mode);
    void setStates(int num_states);
    void setBehavior(ControlButtonMode mode, int num_states);

  private:
    void updateBehavior();
    enum ControlButtonMode m_buttonMode;
    int m_iNoStates;
};
