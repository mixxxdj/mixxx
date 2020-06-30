#include "macromanager.h"

#define MACRORECORDING_PREF_KEY "[MacroRecording]"

MacroManager::MacroManager(
        UserSettingsPointer pConfig,
        EngineMaster* pEngine,
        PlayerManager* pPlayerManager)
        : m_pConfig(pConfig),
          m_pRecordedMacro(new Macro()) {
    qCDebug(macros) << "MacroManager construct";

    m_pToggleRecording = new ControlPushButton(
            ConfigKey(MACRORECORDING_PREF_KEY, "recording_toggle"));
    connect(m_pToggleRecording,
            &ControlPushButton::valueChanged,
            this,
            &MacroManager::slotToggleRecording);
    m_pCORecStatus = new ControlObject(ConfigKey(MACRORECORDING_PREF_KEY, "recording_status"));

    //m_deckCO = new ControlObject(ConfigKey(MACRORECORDING_PREF_KEY, "deck"));

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
    m_pRecordedMacro->dump();
}
