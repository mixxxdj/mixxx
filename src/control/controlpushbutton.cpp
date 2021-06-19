#include "control/controlpushbutton.h"

#include "moc_controlpushbutton.cpp"

/* -------- ------------------------------------------------------
   Purpose: Creates a new simulated latching push-button.
   Input:   key - Key for the configuration file
   -------- ------------------------------------------------------ */
ControlPushButton::ControlPushButton(const ConfigKey& key, bool bPersist, double defaultValue)
        : ControlObject(key, false, false, bPersist, defaultValue),
          m_buttonMode(PUSH),
          m_iNoStates(2) {
    if (m_pControl) {
        m_pControl->setBehavior(
                new ControlPushButtonBehavior(
                        static_cast<ControlPushButtonBehavior::ButtonMode>(m_buttonMode),
                        m_iNoStates));
    }
}

ControlPushButton::~ControlPushButton() {
}

// Tell this PushButton how to act on rising and falling edges
void ControlPushButton::setButtonMode(enum ButtonMode mode) {
    //qDebug() << "Setting " << m_Key.group << m_Key.item << "as toggle";
    m_buttonMode = mode;

    if (m_pControl) {
        m_pControl->setBehavior(
                new ControlPushButtonBehavior(
                        static_cast<ControlPushButtonBehavior::ButtonMode>(m_buttonMode),
                        m_iNoStates));
    }
}

void ControlPushButton::setStates(int num_states) {
    m_iNoStates = num_states;

    if (m_pControl) {
            m_pControl->setBehavior(
                    new ControlPushButtonBehavior(
                            static_cast<ControlPushButtonBehavior::ButtonMode>(m_buttonMode),
                            m_iNoStates));
    }
}
