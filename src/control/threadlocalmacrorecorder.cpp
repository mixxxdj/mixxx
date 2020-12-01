#include "control/threadlocalmacrorecorder.h"

#include "control/controlpushbutton.h"
#include "control/macrorecorder.h"

//static
QSharedPointer<ThreadLocalMacroRecorder> ThreadLocalMacroRecorder::s_pMacroRecorder;

ThreadLocalMacroRecorder::ThreadLocalMacroRecorder(QObject* parent)
        : QObject(parent),
          m_pCoRecording(ConfigKey("[MacroRecorder]", "recording")) {
}

MacroRecorder* ThreadLocalMacroRecorder::get() {
    if (!m_pMacroRecorder.hasLocalData()) {
        m_pMacroRecorder.setLocalData(new MacroRecorder(this));
    }
    return m_pMacroRecorder.localData();
}
