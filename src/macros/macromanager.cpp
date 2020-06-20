#include "macromanager.h"

MacroManager::MacroManager(
        UserSettingsPointer pConfig,
        EngineMaster* pEngine,
        PlayerManager* pPlayerManager)
        : m_pConfig(pConfig) {
    qDebug() << "MacroManager init";
    auto hotcueActivate = new ControlProxy("[Channel1]", "hotcue_1_activate", this);
    hotcueActivate->connectValueChanged(
            this, &MacroManager::slotHotcueActivate, Qt::DirectConnection);
};

void MacroManager::slotHotcueActivate(double v) {
    qDebug() << "MacroManager: HOTCUE 1 ACTIVATED WITH VALUE " << toDebugString(v);
}
