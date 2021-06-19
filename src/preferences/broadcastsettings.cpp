#include "preferences/broadcastsettings.h"

#include <QDir>
#include <QFileInfo>
#include <QFileInfoList>
#include <QStringList>

#include "broadcast/defs_broadcast.h"
#include "defs_urls.h"
#include "moc_broadcastsettings.cpp"
#include "util/logger.h"
#include "util/memory.h"

namespace {
const char* kProfilesSubfolder = "broadcast_profiles";
const char* kDefaultProfile = "Connection 1"; // Must be used only when initializing profiles
const mixxx::Logger kLogger("BroadcastSettings");
} // anonymous namespace

BroadcastSettings::BroadcastSettings(
        UserSettingsPointer pConfig, QObject* parent)
    : QObject(parent),
      m_pConfig(pConfig),
      m_profiles() {
    loadProfiles();
}

void BroadcastSettings::loadProfiles() {
    QDir profilesFolder(getProfilesFolder());
    if (!profilesFolder.exists()) {
        kLogger.info() << "Profiles folder doesn't exist. Creating it.";
        profilesFolder.mkpath(profilesFolder.absolutePath());
    }

    QStringList nameFilters("*.bcp.xml");
    QFileInfoList files =
            profilesFolder.entryInfoList(nameFilters, QDir::Files, QDir::Name);

    // If *.bcp.xml files exist in the profiles subfolder, those will be loaded
    // and instantiated in the class' internal profile list for other by it and
    // Mixxx subsystems related to Live Broadcasting.
    // If that directory is empty (common reasons: it has been created by the
    // code at the beginning, or all profiles were deleted) then a default
    // profile with default values is created. That case could also mean that
    // Mixxx has just been upgraded to a new version, so "legacy format" values
    // has fetched from mixxx.cfg and loaded into the fresh default profile.
    // It's important to take into account that the "legacy" settings are left
    // in mixxx.cfg for retro-compatibility during alpha and beta testing.

    if (files.size() > 0) {
        kLogger.info() << "Found" << files.size() << "profile(s)";

        // Load profiles from filesystem
        for (const QFileInfo& fileInfo : files) {
            BroadcastProfilePtr profile =
                    BroadcastProfile::loadFromFile(fileInfo.absoluteFilePath());

            if (profile) {
                addProfile(profile);
            }
        }
    } else {
        kLogger.info() << "No profiles found. Creating default profile.";

        BroadcastProfilePtr defaultProfile(
                    new BroadcastProfile(kDefaultProfile));
        // Upgrade from mixxx.cfg format to XML (if required)
        loadLegacySettings(defaultProfile);

        addProfile(defaultProfile);
        saveProfile(defaultProfile);
    }
}

bool BroadcastSettings::addProfile(BroadcastProfilePtr profile) {
    if (!profile) {
        return false;
    }

    if (m_profiles.size() >= BROADCAST_MAX_CONNECTIONS) {
        kLogger.warning() << "addProfile: connection limit reached."
                 << "can't add more than" << QString::number(BROADCAST_MAX_CONNECTIONS)
                 << "connections.";
        return false;
    }

    // It is best to avoid using QSharedPointer::data(), especially when
    // passing it to another function, as it puts the associated pointer
    // at risk of being manually deleted.
    // However it's fine with Qt's connect because it can be trusted that
    // it won't delete the pointer.
    connect(profile.data(),
            &BroadcastProfile::profileNameChanged,
            this,
            &BroadcastSettings::onProfileNameChanged);
    connect(profile.data(),
            &BroadcastProfile::connectionStatusChanged,
            this,
            &BroadcastSettings::onConnectionStatusChanged);
    m_profiles.insert(profile->getProfileName(), BroadcastProfilePtr(profile));

    emit profileAdded(profile);
    return true;
}

bool BroadcastSettings::saveProfile(BroadcastProfilePtr profile) {
    if (!profile) {
        return false;
    }

    QString filename = profile->getLastFilename();
    if (filename.isEmpty()) {
        // there was no previous filename, find an unused filename
        for (int index = 0;; ++index) {
            if (index > 0) {
                // add an index to the file name to make it unique
                filename = filePathForProfile(profile->getProfileName() + QString::number(index));
            } else {
                filename = filePathForProfile(profile->getProfileName());
            }

            QFileInfo fileInfo(filename);
            if (!fileInfo.exists()) {
                break;
            }
        }
    }

    return profile->save(filename);
}

QString BroadcastSettings::filePathForProfile(const QString& profileName) {
    QString filename = profileName + ".bcp.xml";
    filename = BroadcastProfile::stripForbiddenChars(filename);
    return QDir(getProfilesFolder()).absoluteFilePath(filename);
}

QString BroadcastSettings::filePathForProfile(BroadcastProfilePtr profile) {
    if (!profile) {
        return QString();
    }

    return filePathForProfile(profile->getProfileName());
}

bool BroadcastSettings::deleteFileForProfile(BroadcastProfilePtr profile) {
    if (!profile) {
        return false;
    }

    QString filename = profile->getLastFilename();
    if (filename.isEmpty()) {
        // no file was saved, there is no file to delete
        return false;
    }

    QFileInfo xmlFile(filename);
    if (xmlFile.exists()) {
        return QFile::remove(xmlFile.absoluteFilePath());
    }
    return false;
}

QString BroadcastSettings::getProfilesFolder() {
    QString profilesPath(m_pConfig->getSettingsPath());
    profilesPath.append(QDir::separator() + QString(kProfilesSubfolder));
    return profilesPath;
}

void BroadcastSettings::saveAll() {
    for (const auto& kv : qAsConst(m_profiles)) {
        saveProfile(kv);
    }
    emit profilesChanged();
}

void BroadcastSettings::onProfileNameChanged(const QString& oldName, const QString& newName) {
    if (!m_profiles.contains(oldName)) {
        return;
    }

    BroadcastProfilePtr profile = m_profiles.take(oldName);
    if (profile) {
        m_profiles.insert(newName, profile);
        emit profileRenamed(oldName, profile);

        deleteFileForProfile(profile);
        saveProfile(profile);
    }
}

void BroadcastSettings::onConnectionStatusChanged(int newStatus) {
    Q_UNUSED(newStatus);
}

BroadcastProfilePtr BroadcastSettings::profileAt(int index) {
    auto it = m_profiles.begin() + index;
    return it != m_profiles.end() ? it.value() : BroadcastProfilePtr(nullptr);
}

QList<BroadcastProfilePtr> BroadcastSettings::profiles() {
    return m_profiles.values();
}

void BroadcastSettings::applyModel(BroadcastSettingsModel* pModel) {
    if (!pModel) {
        return;
    }
    // TODO(Palakis): lock both lists against modifications while syncing

    // Step 1: find profiles to delete from the settings
    for (auto profileIter = m_profiles.begin(); profileIter != m_profiles.end();) {
        QString profileName = (*profileIter)->getProfileName();
        if (!pModel->getProfileByName(profileName)) {
            // If profile exists in settings but not in the model,
            // remove the profile from the settings
            const auto removedProfile = *profileIter;
            deleteFileForProfile(removedProfile);
            profileIter = m_profiles.erase(profileIter);
            emit profileRemoved(removedProfile);
        } else {
            ++profileIter;
        }
    }

    // Step 2: add new profiles
    const QList<BroadcastProfilePtr> existingProfiles = pModel->profiles();
    for (auto profileCopy : existingProfiles) {
        // Check if profile already exists in settings
        BroadcastProfilePtr existingProfile =
                m_profiles.value(profileCopy->getProfileName());
        if (!existingProfile) {
            // If no profile with the same name exists, add the new
            // profile to the settings.
            // The BroadcastProfile instance is a copy to separate it
            // from its existence in the model
            addProfile(profileCopy->valuesCopy());
        }
    }

    // Step 3: update existing profiles
    const QList<BroadcastProfilePtr> allProfiles = pModel->profiles();
    for (BroadcastProfilePtr profileCopy : allProfiles) {
        BroadcastProfilePtr actualProfile =
                m_profiles.value(profileCopy->getProfileName());
        if (actualProfile) {
            profileCopy->copyValuesTo(actualProfile);
        }
    }

    saveAll();
}
