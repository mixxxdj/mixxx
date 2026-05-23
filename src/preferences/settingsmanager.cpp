#include "preferences/settingsmanager.h"

#include <QDir>

#include "control/control.h"
#include "preferences/upgrade.h"
#include "util/assert.h"

#ifdef __BUNGEE__
#include "engine/enginebuffer.h"
#endif

SettingsManager::SettingsManager(const QString& settingsPath)
        : m_bShouldRescanLibrary(false) {
    // First make sure the settings path exists. If we don't then other parts of
    // Mixxx (such as the library) will produce confusing errors.
    const bool settingsDirectoryExistedBeforeStartup = QDir(settingsPath).exists();
    if (!settingsDirectoryExistedBeforeStartup) {
        QDir().mkpath(settingsPath);
    }

    // Check to see if this is the first time this version of Mixxx is run
    // after an upgrade and make any needed changes.
    Upgrade upgrader;
    m_pSettings = upgrader.versionUpgrade(settingsPath);
    VERIFY_OR_DEBUG_ASSERT(!m_pSettings.isNull()) {
        m_pSettings = UserSettingsPointer(new UserSettings(""));
    }

#ifdef __BUNGEE__
    if (!settingsDirectoryExistedBeforeStartup) {
        const ConfigKey keylockEngineKey(
                QStringLiteral("[App]"),
                QStringLiteral("keylock_engine"));
        if (!m_pSettings->exists(keylockEngineKey)) {
            m_pSettings->setValue(
                    keylockEngineKey,
                    EngineBuffer::KeylockEngine::Bungee);
        }
    }
#endif

    m_bShouldRescanLibrary = upgrader.rescanLibrary();

    ControlDoublePrivate::setUserConfig(m_pSettings);

#ifdef __BROADCAST__
    m_pBroadcastSettings = BroadcastSettingsPointer(
                               new BroadcastSettings(m_pSettings));
#endif
}

SettingsManager::~SettingsManager() {
    ControlDoublePrivate::setUserConfig(UserSettingsPointer());
}
