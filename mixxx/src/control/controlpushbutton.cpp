#include "control/controlpushbutton.h"

#include "moc_controlpushbutton.cpp"

/* -------- ------------------------------------------------------
   Purpose: Creates a new simulated latching push-button.
   Input:   key - Key for the configuration file
   -------- ------------------------------------------------------ */
ControlPushButton::ControlPushButton(const ConfigKey& key, bool bPersist, double defaultValue)
        : ControlObject(key, false, false, bPersist, defaultValue),
          m_buttonMode(mixxx::control::ButtonMode::Push),
          m_iNoStates(2) {
    updateBehavior();
}

ControlPushButton::~ControlPushButton() = default;

void ControlPushButton::setButtonMode(mixxx::control::ButtonMode mode) {
    if (m_buttonMode != mode) {
        m_buttonMode = mode;
        updateBehavior();
    }
}

void ControlPushButton::setStates(int num_states) {
    if (m_iNoStates != num_states) {
        m_iNoStates = num_states;
        updateBehavior();
    }
}

void ControlPushButton::setBehavior(mixxx::control::ButtonMode mode, int num_states) {
    bool shouldUpdate = false;
    if (m_buttonMode != mode) {
        m_buttonMode = mode;
        shouldUpdate = true;
    }
    if (m_iNoStates != num_states) {
        m_iNoStates = num_states;
        shouldUpdate = true;
    }

    // If we would update unconditional, the state would be set always to the default value
    if (shouldUpdate) {
        updateBehavior();
    }
}

// private
void ControlPushButton::updateBehavior() {
    if (m_pControl) {
        m_pControl->setBehavior(
                new ControlPushButtonBehavior(
                        m_buttonMode,
                        m_iNoStates));
    }
}
