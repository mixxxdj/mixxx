#include <QDir>
#include <QStringList>
#include <QFileInfoList>
#include <QFileInfo>

#include "util/logger.h"
#include "preferences/broadcastsettings.h"
#include "broadcast/defs_broadcast.h"
#include "defs_urls.h"

namespace {
const char* kConfigKey = "[Shoutcast]";
const char* kCurrentProfile = "current_profile";
const char* kProfilesSubfolder = "broadcast_profiles";
const char* kDefaultProfile = "Default Profile";
const mixxx::Logger kLogger("BroadcastSettings");
} // anonymous namespace

BroadcastSettings::BroadcastSettings(
        UserSettingsPointer pConfig, QObject* parent)
    : QObject(parent),
      m_pConfig(pConfig),
      m_profiles(),
      m_currentProfile(kDefaultProfile) {
    loadProfiles();
}

void BroadcastSettings::loadProfiles() {
    QDir profilesFolder(getProfilesFolder());
    if(!profilesFolder.exists()) {
        kLogger.debug()
                << "Profiles folder doesn't exist. Creating it.";
        profilesFolder.mkpath(profilesFolder.absolutePath());
    }

    QStringList nameFilters("*.bcp.xml");
    QFileInfoList files =
            profilesFolder.entryInfoList(nameFilters, QDir::Files, QDir::Name);

    // If *.bcp.xml files exist in the profiles subfolder, those will be loaded
    // and instanciated in the class' internal profile list for other by it and
    // Mixxx subsystems related to Live Broadcasting.
    // If that directory is empty (common reasons: it has been created by the
    // code at the beginning, or all profiles were deleted) then a default
    // profile with default values is created. That case could also mean that
    // Mixxx has just been upgraded to a new version, so "legacy format" values
    // has fetched from mixxx.cfg and loaded into the fresh default profile.
    // It's important to take into account that the "legacy" settings are left
    // in mixxx.cfg for retro-compatibility during alpha and beta testing.

    if(files.count() > 0) {
        kLogger.debug() << "Found " << files.count() << " profiles.";

        // Load profiles from filesystem
        for(QFileInfo fileInfo : files) {
            BroadcastProfilePtr profile =
                    BroadcastProfile::loadFromFile(fileInfo.absoluteFilePath());

            if(profile)
                m_profiles[profile->getProfileName()] = std::move(profile);
        }
    } else {
        kLogger.debug() << "No profiles found. Creating default profile.";

        BroadcastProfilePtr defaultProfile =
                std::make_unique<BroadcastProfile>(kDefaultProfile);

        // Upgrade from mixxx.cfg format to XML (if required)
        loadLegacySettings(defaultProfile);

        saveProfile(defaultProfile);

        m_profiles[defaultProfile->getProfileName()] =
                std::move(defaultProfile);
        setCurrentProfile(defaultProfile);
    }
}

void BroadcastSettings::saveProfile(const BroadcastProfilePtr& profile) {
    if(!profile)
        return;

    profile->save(filePathForProfile(profile));
}

QString BroadcastSettings::filePathForProfile(const QString& profileName) {
    QString filename = profileName + ".bcp.xml";
    filename = BroadcastProfile::stripForbiddenChars(filename);
    return QDir(getProfilesFolder()).absoluteFilePath(filename);
}

QString BroadcastSettings::filePathForProfile(
        const BroadcastProfilePtr& profile) {
    if(!profile)
        return QString();

    return filePathForProfile(profile->getProfileName());
}

QString BroadcastSettings::getProfilesFolder() {
    QString profilesPath(m_pConfig->getSettingsPath());
    profilesPath.append(QDir::separator() + QString(kProfilesSubfolder));
    return profilesPath;
}

void BroadcastSettings::setCurrentProfile(const BroadcastProfilePtr& profile) {
    if(!profile)
        return;

    QString profileName = profile->getProfileName();
    m_pConfig->setValue(ConfigKey(kConfigKey, kCurrentProfile), profileName);
}

const BroadcastProfilePtr& BroadcastSettings::getCurrentProfile() {
    QString currentProfile = m_pConfig->getValue(
                                 ConfigKey(kConfigKey, kCurrentProfile),
                                 kDefaultProfile);
    return getProfileByName(currentProfile);
}

const BroadcastProfilePtr& BroadcastSettings::getProfileByName(
        const QString& profileName) {
    return m_profiles[profileName];
}

void BroadcastSettings::saveAll() {
    for(auto& kv : m_profiles) {
        saveProfile(kv.second);
    }
}

void BroadcastSettings::deleteProfile(const BroadcastProfilePtr& profile) {
    if(!profile)
        return;

    QFileInfo xmlFile(filePathForProfile(profile));
    if(xmlFile.exists())
        QFile::remove(xmlFile.absolutePath());

    m_profiles.erase(profile->getProfileName());
}

void BroadcastSettings::onProfileNameChanged(QString oldName, QString newName) {
    if(m_profiles[oldName]) {
        BroadcastProfilePtr oldItem = std::move(m_profiles[oldName]);
        m_profiles.erase(oldName);

        m_profiles[newName] = std::move(oldItem);
    }
}
