#include "macromanager.h"

#define MACRORECORDING_PREF_KEY "[MacroRecording]"

MacroManager::MacroManager(
        UserSettingsPointer pConfig,
        EngineMaster* pEngine,
        PlayerManager* pPlayerManager)
        : m_pConfig(pConfig) {
    qDebug() << "MacroManager init";
    m_hotcueActivate = new ControlProxy("[Channel1]", "hotcue_1_activate", this);
    m_hotcueActivate->connectValueChanged(
            this, &MacroManager::slotHotcueActivate, Qt::DirectConnection);

    m_pToggleRecording = new ControlPushButton(
            ConfigKey(MACRORECORDING_PREF_KEY, "recording_toggle"));
    connect(m_pToggleRecording,
            &ControlPushButton::valueChanged,
            this,
            &MacroManager::slotToggleRecording);
    m_recStatusCO = new ControlObject(ConfigKey(MACRORECORDING_PREF_KEY, "recording_status"));
    m_recStatus = new ControlProxy(m_recStatusCO->getKey(), this);

    //m_deckCO = new ControlObject(ConfigKey(MACRORECORDING_PREF_KEY, "deck"));
    //m_deck = new ControlProxy(m_recStatusCO->getKey(), this);
}

MacroManager::~MacroManager() {
    qDebug() << "Delete MacroManager";

    delete m_recStatusCO;
    delete m_pToggleRecording;
}

void MacroManager::slotHotcueActivate(double v) {
    qDebug() << "MacroManager: HOTCUE 1 ACTIVATED WITH VALUE " << toDebugString(v);
}

void MacroManager::startRecording() {
    qDebug() << "MacroManager start recording";
    m_recStatus->set(1);
    m_bRecording = true;
}

void MacroManager::stopRecording() {
    qDebug() << "MacroManager stop recording";
    m_recStatus->set(0);
    m_bRecording = false;
}
