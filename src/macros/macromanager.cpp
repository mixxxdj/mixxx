#include "macromanager.h"

MacroManager::MacroManager(
        UserSettingsPointer pConfig,
        EngineMaster* pEngine,
        PlayerManager* pPlayerManager)
        : m_pConfig(pConfig) {
    qDebug() << "MacroManager init";
    auto hotcueActivate = new ControlPushButton(ConfigKey("[Channel1]", "hotcue_1_activate"));
    connect(hotcueActivate,
            &ControlObject::valueChanged,
            this,
            &MacroManager::slotHotcueActivate,
            Qt::DirectConnection);
};

void MacroManager::slotHotcueActivate(double v) {
    qDebug() << "MacroManager: HOTCUE 1 ACTIVATED WITH VALUE " << toDebugString(v);
}
