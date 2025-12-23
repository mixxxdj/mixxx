#pragma once

#include "control/controlbuttonmode.h"
#include "control/controlobject.h"

class ControlPushButton : public ControlObject {
    Q_OBJECT
  public:
    static QString buttonModeToString(mixxx::control::ButtonMode buttonMode) {
        switch (buttonMode) {
        case mixxx::control::ButtonMode::Push:
            return QStringLiteral("Push");
        case mixxx::control::ButtonMode::Toggle:
            return QStringLiteral("Toggle");
        case mixxx::control::ButtonMode::PowerWindow:
            return QStringLiteral("PowerWindow");
        case mixxx::control::ButtonMode::LongPressLatching:
            return QStringLiteral("LongPressLatching");
        case mixxx::control::ButtonMode::Trigger:
            return QStringLiteral("Trigger");
        }
        DEBUG_ASSERT(false);
        return "Unknown";
    }

    ControlPushButton(const ConfigKey& key, bool bPersist = false, double defaultValue = 0.0);
    virtual ~ControlPushButton();

    inline mixxx::control::ButtonMode getButtonMode() const {
        return m_buttonMode;
    }
    void setButtonMode(mixxx::control::ButtonMode mode);
    void setStates(int num_states);
    void setBehavior(mixxx::control::ButtonMode mode, int num_states);

  private:
    void updateBehavior();
    enum mixxx::control::ButtonMode m_buttonMode;
    int m_iNoStates;
};
