#include "control/threadlocalmacrorecorder.h"

#include "control/controlpushbutton.h"
#include "control/macrorecorder.h"

//static
std::shared_ptr<ThreadLocalMacroRecorder> ThreadLocalMacroRecorder::s_pMacroRecorder;

ThreadLocalMacroRecorder::ThreadLocalMacroRecorder(QObject* parent)
        : QObject(parent),
          m_coRecording(ConfigKey("[MacroRecorder]", "recording")),
          m_coTrigger(ConfigKey("[MacroRecorder]", "trigger")) {
    m_coRecording.setButtonMode(ControlPushButton::TOGGLE);
    m_coTrigger.setButtonMode(ControlPushButton::TRIGGER);
    connect(&m_coTrigger,
            &ControlObject::valueChanged,
            this,
            &ThreadLocalMacroRecorder::slotTriggered);
}

MacroRecorder* ThreadLocalMacroRecorder::get() {
    if (!m_pMacroRecorder.hasLocalData()) {
        m_pMacroRecorder.setLocalData(new MacroRecorder(this));
    }
    return m_pMacroRecorder.localData();
}

void ThreadLocalMacroRecorder::slotTriggered(double) {
    m_coRecording.set(0);
}
