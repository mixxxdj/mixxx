#include "preferences/settingsmanager.h"

#include <QDir>

#include "control/control.h"
#include "preferences/upgrade.h"
#include "util/assert.h"

SettingsManager::SettingsManager(QObject* pParent,
                                 const QString& settingsPath)
        : QObject(pParent),
          m_bShouldRescanLibrary(false) {
    // First make sure the settings path exists. If we don't then other parts of
    // Mixxx (such as the library) will produce confusing errors.
    if (!QDir(settingsPath).exists()) {
        QDir().mkpath(settingsPath);
    }

    // Check to see if this is the first time this version of Mixxx is run
    // after an upgrade and make any needed changes.
    Upgrade upgrader;
    m_pSettings = upgrader.versionUpgrade(settingsPath);
    DEBUG_ASSERT_AND_HANDLE(!m_pSettings.isNull()) {
        m_pSettings = UserSettingsPointer(new UserSettings(""));
    }
    m_bShouldRescanLibrary = upgrader.rescanLibrary();
    ControlDoublePrivate::setUserConfig(m_pSettings);
}

SettingsManager::~SettingsManager() {
    ControlDoublePrivate::setUserConfig(UserSettingsPointer());
}

void SettingsManager::initializeDefaults() {
    QString resourcePath = m_pSettings->getResourcePath();

    // Store the last resource path in the config database.
    // TODO(rryan): this looks unused.
    m_pSettings->set(ConfigKey("[Config]", "Path"), ConfigValue(resourcePath));

    // Do not write meta data back to ID3 when meta data has changed
    // Because multiple TrackDao objects can exists for a particular track
    // writing meta data may ruin your MP3 file if done simultaneously.
    // see Bug #728197
    // For safety reasons, we deactivate this feature.
    m_pSettings->set(ConfigKey("[Library]","WriteAudioTags"), ConfigValue(0));

    // Intialize default BPM system values.
    // NOTE(rryan): These should be in a better place but they've always been in
    // MixxxMainWindow.
    if (!m_pSettings->exists(ConfigKey("[BPM]", "BPMRangeStart"))) {
        m_pSettings->set(ConfigKey("[BPM]", "BPMRangeStart"),ConfigValue(65));
    }

    if (!m_pSettings->exists(ConfigKey("[BPM]", "BPMRangeEnd"))) {
        m_pSettings->set(ConfigKey("[BPM]", "BPMRangeEnd"),ConfigValue(135));
    }

    if (!m_pSettings->exists(ConfigKey("[BPM]", "AnalyzeEntireSong"))) {
        m_pSettings->set(ConfigKey("[BPM]", "AnalyzeEntireSong"),ConfigValue(1));
    }

}
