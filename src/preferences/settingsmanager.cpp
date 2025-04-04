#include "preferences/settingsmanager.h"

#include <QDir>

#include "control/control.h"
#include "preferences/upgrade.h"
#include "util/assert.h"

void displaySettingsPermError(const QString& settingsPath, const QString& operation) {
    QMessageBox::critical(nullptr,
            QObject::tr("Cannot access settings folder"),
            QObject::tr("Mixxx cannot access the settings folder at\n%1\n"
                        "because you do not have permission "
                        "to %2 files in that folder.\n"
                        "Change permissions for the settings folder "
                        "or specify a different folder by running "
                        "Mixxx with the --settingsPath option.\n\n"
                        "Click OK to exit.")
                    .arg(settingsPath, operation),
            QMessageBox::Ok);
}

bool checkSettingsPathPerm(const QString& settingsPath) {
    QFile file(settingsPath + "test_perm_file");
    if (!file.open(QIODevice::WriteOnly)) {
        displaySettingsPermError(settingsPath, "write");
        file.remove();
        return false;
    }
    file.close();
    if (!file.open(QIODevice::ReadOnly)) {
        displaySettingsPermError(settingsPath, "read");
        file.remove();
        return false;
    }
    file.remove();
    return true;
}

SettingsManager::SettingsManager(const QString& settingsPath)
        : m_bShouldRescanLibrary(false) {
    // First make sure the settings path exists. If we don't then other parts of
    // Mixxx (such as the library) will produce confusing errors.
    if (!QDir(settingsPath).exists()) {
        QDir().mkpath(settingsPath);
    }

    if (!checkSettingsPathPerm(settingsPath)) {
        exit(EXIT_FAILURE);
    }

    // Check to see if this is the first time this version of Mixxx is run
    // after an upgrade and make any needed changes.
    Upgrade upgrader;
    m_pSettings = upgrader.versionUpgrade(settingsPath);
    VERIFY_OR_DEBUG_ASSERT(!m_pSettings.isNull()) {
        m_pSettings = UserSettingsPointer(new UserSettings(""));
    }
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
