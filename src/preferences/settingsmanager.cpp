#include "preferences/settingsmanager.h"

#include "control/control.h"
#include "upgrade.h"
#include "util/assert.h"

SettingsManager::SettingsManager(QObject* pParent,
                                 const QString& settingsPath)
        : QObject(pParent),
          m_bShouldRescanLibrary(false) {
    // Check to see if this is the first time this version of Mixxx is run
    // after an upgrade and make any needed changes.
    Upgrade upgrader;
    UserSettings* pSettings = upgrader.versionUpgrade(settingsPath);
    DEBUG_ASSERT_AND_HANDLE(pSettings != nullptr) {
        pSettings = new UserSettings("");
    }
    m_pSettings = UserSettingsPointer(pSettings);
    m_bShouldRescanLibrary = upgrader.rescanLibrary();
    ControlDoublePrivate::setUserConfig(m_pSettings);
}

SettingsManager::~SettingsManager() {
    ControlDoublePrivate::setUserConfig(UserSettingsPointer());
}
