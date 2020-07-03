#include "macromanager.h"

// TODO(xerus) handle track eject while recording
// TODO(xerus) write tests

/// The MacroManager handles the recording of Macros and the [MacroRecording] controls.
MacroManager::MacroManager()
        : m_COToggleRecording(ControlPushButton(ConfigKey(kMacroRecordingKey, "recording_toggle"))),
          m_CORecStatus(ControlObject(ConfigKey(kMacroRecordingKey, "recording_status"))),
          m_CODeck(ControlObject(ConfigKey(kMacroRecordingKey, "deck"))),
          m_pRecordedMacro(new Macro()) {
    qCDebug(macros) << "MacroManager construct";

    connect(&m_COToggleRecording,
            &ControlPushButton::valueChanged,
            this,
            &MacroManager::slotToggleRecording);
}

void MacroManager::startRecording() {
    qCDebug(macros) << "MacroManager recording start";
    m_bRecording = true;
    m_CORecStatus.set(1);
    m_pRecordedMacro->clear();
    emit startMacroRecording(m_pRecordedMacro);
}

void MacroManager::stopRecording() {
    qCDebug(macros) << "MacroManager recording stop";
    m_bRecording = false;
    m_CORecStatus.set(0);
    emit stopMacroRecording();
    // TODO(xerus) wait until stopped
    qCDebug(macros) << "Recorded Macro for deck" << m_CODeck.get();
    m_pRecordedMacro->dump();
}
