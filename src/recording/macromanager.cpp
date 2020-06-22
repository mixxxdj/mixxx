#include "macromanager.h"

#define MACRORECORDING_PREF_KEY "[MacroRecording]"

MacroManager::MacroManager(
        UserSettingsPointer pConfig,
        EngineMaster* pEngine,
        PlayerManager* pPlayerManager)
        : m_pConfig(pConfig) {
    qDebug() << "MacroManager init";
    m_pCPHotcueActivate = new ControlProxy("[Channel1]", "hotcue_1_activate", this);
    m_pCPHotcueActivate->connectValueChanged(
            this, &MacroManager::slotHotcueActivate, Qt::DirectConnection);

    m_pToggleRecording = new ControlPushButton(
            ConfigKey(MACRORECORDING_PREF_KEY, "recording_toggle"));
    connect(m_pToggleRecording,
            &ControlPushButton::valueChanged,
            this,
            &MacroManager::slotToggleRecording);
    m_pCORecStatus = new ControlObject(ConfigKey(MACRORECORDING_PREF_KEY, "recording_status"));
    m_pCPRecStatus = new ControlProxy(m_pCORecStatus->getKey(), this);

    connect(this,
            &MacroManager::startMacroRecording,
            pEngine,
            &EngineMaster::slotStartMacroRecording);
    connect(this,
            &MacroManager::stopMacroRecording,
            pEngine,
            &EngineMaster::slotStopMacroRecording);
    //m_deckCO = new ControlObject(ConfigKey(MACRORECORDING_PREF_KEY, "deck"));
    //m_deck = new ControlProxy(m_recStatusCO->getKey(), this);
}

MacroManager::~MacroManager() {
    qDebug() << "Delete MacroManager";

    delete m_pCORecStatus;
    delete m_pToggleRecording;
}

void MacroManager::slotHotcueActivate(double v) {
    qDebug() << "MacroManager: HOTCUE 1 ACTIVATED WITH VALUE " << toDebugString(v);
}

void MacroManager::startRecording() {
    qDebug() << "MacroManager start recording";
    m_bRecording = true;
    m_pCPRecStatus->set(1);
    m_pRecordedMacro->clear();
    emit startMacroRecording(m_pRecordedMacro);
}

void MacroManager::stopRecording() {
    qDebug() << "MacroManager stop recording";
    m_bRecording = false;
    m_pCPRecStatus->set(0);
    emit stopMacroRecording();
    m_pRecordedMacro->dump();
}
