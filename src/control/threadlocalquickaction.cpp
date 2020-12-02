#include "control/threadlocalquickaction.h"

#include "control/controlpushbutton.h"
#include "control/quickaction.h"

//static
std::shared_ptr<ThreadLocalQuickAction> ThreadLocalQuickAction::s_pMacroRecorder;

ThreadLocalQuickAction::ThreadLocalQuickAction(QObject* parent)
        : QObject(parent),
          m_coRecording(ConfigKey("[QuickAction]", "recording")),
          m_coTrigger(ConfigKey("[QuickAction]", "trigger")) {
    m_coRecording.setButtonMode(ControlPushButton::TOGGLE);
    m_coTrigger.setButtonMode(ControlPushButton::TRIGGER);
    connect(&m_coTrigger,
            &ControlObject::valueChanged,
            this,
            &ThreadLocalQuickAction::slotTriggered);
}

QuickAction* ThreadLocalQuickAction::get() {
    if (!m_pMacroRecorder.hasLocalData()) {
        m_pMacroRecorder.setLocalData(new QuickAction(this));
    }
    return m_pMacroRecorder.localData();
}

void ThreadLocalQuickAction::slotTriggered(double) {
    m_coRecording.set(0);
}
