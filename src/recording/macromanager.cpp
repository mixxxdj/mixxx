#include "macromanager.h"

// TODO(xerus) handle track eject while recording
// TODO(xerus) write tests
// TODO(xerus) consider how to record the first jump

/// The MacroManager handles the recording of Macros and the [MacroRecording] controls.
MacroManager::MacroManager(EngineMaster* pEngine)
        : m_pRecordedMacro(new Macro()) {
    qCDebug(macros) << "MacroManager construct";

    m_pToggleRecording = new ControlPushButton(
            ConfigKey(kMacroRecordingKey, "recording_toggle"));
    connect(m_pToggleRecording,
            &ControlPushButton::valueChanged,
            this,
            &MacroManager::slotToggleRecording);
    m_pCORecStatus = new ControlObject(
            ConfigKey(kMacroRecordingKey, "recording_status"));

    m_pCODeck = new ControlObject(ConfigKey(kMacroRecordingKey, "deck"));

    connect(this,
            &MacroManager::startMacroRecording,
            pEngine,
            &EngineMaster::slotStartMacroRecording);
    connect(this,
            &MacroManager::stopMacroRecording,
            pEngine,
            &EngineMaster::slotStopMacroRecording);
}

MacroManager::~MacroManager() {
    qCDebug(macros) << "MacroManager deconstruct";
    delete m_pCORecStatus;
    delete m_pToggleRecording;
}

void MacroManager::startRecording() {
    qCDebug(macros) << "MacroManager recording start";
    m_bRecording = true;
    m_pCORecStatus->set(1);
    m_pRecordedMacro->clear();
    emit startMacroRecording(m_pRecordedMacro);
}

void MacroManager::stopRecording() {
    qCDebug(macros) << "MacroManager recording stop";
    m_bRecording = false;
    m_pCORecStatus->set(0);
    emit stopMacroRecording();
    // TODO(xerus) wait until stopped
    qCDebug(macros) << "Recorded Macro for deck" << m_pCODeck->get();
    m_pRecordedMacro->dump();
}
