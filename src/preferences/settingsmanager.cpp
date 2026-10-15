#include "preferences/settingsmanager.h"

#include <QDir>

#include "control/control.h"
#include "preferences/upgrade.h"
#include "util/assert.h"

#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
#include <QNtfsPermissionCheckGuard>
#else
extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;
#endif

void displaySettingsPermError(const QString& settingsPath) {
    QMessageBox::critical(nullptr,
            QObject::tr("Cannot access settings folder"),
            QObject::tr("Mixxx cannot access the settings folder at\n%1\n"
                        "because you do not have permission "
                        "to read/write files in that folder.\n"
                        "Change permissions for the settings folder "
                        "or specify a different folder by running "
                        "Mixxx with the --settingsPath option.\n\n"
                        "Click OK to exit.")
                    .arg(settingsPath),
            QMessageBox::Ok);
}

bool checkSettingsPathPerm(const QString& settingsPath) {
#if defined(Q_OS_WIN)
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
    QNtfsPermissionCheckGuard guard;
#else
    ++qt_ntfs_permission_lookup;
#endif
#endif

    QFileInfo file(settingsPath);
    if (!file.permission(QFileDevice::WriteUser | QFileDevice::ReadUser)) {
        displaySettingsPermError(settingsPath);
        return false;
    }

#if defined(Q_OS_WIN) && QT_VERSION < QT_VERSION_CHECK(6, 6, 0)
    --qt_ntfs_permission_lookup;
#endif
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
