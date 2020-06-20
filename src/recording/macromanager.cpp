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

    //m_pToggleRecording = new ControlPushButton(ConfigKey(MACRORECORDING_PREF_KEY, "toggle_recording"));
    //connect(m_pToggleRecording,
    //        &ControlPushButton::valueChanged,
    //        this,
    //        &MacroManager::slotToggleRecording);
    //m_recStatusCO = new ControlObject(ConfigKey(MACRORECORDING_PREF_KEY, "status"));
    //m_recStatus = new ControlProxy(m_recStatusCO->getKey(), this);
}

void MacroManager::slotHotcueActivate(double v) {
    qDebug() << "MacroManager: HOTCUE 1 ACTIVATED WITH VALUE " << toDebugString(v);
}

void MacroManager::slotSetRecording(bool recording) {
    if (recording && !isRecordingActive()) {
        startRecording();
    } else if (!recording && isRecordingActive()) {
        stopRecording();
    }
}

void MacroManager::slotToggleRecording(double value) {
    bool toggle = static_cast<bool>(value);
    if (toggle) {
        if (isRecordingActive()) {
            stopRecording();
        } else {
            startRecording();
        }
    }
}

bool MacroManager::isRecordingActive() const {
    return false;
}

void MacroManager::startRecording() {
}

void MacroManager::stopRecording() {
}
