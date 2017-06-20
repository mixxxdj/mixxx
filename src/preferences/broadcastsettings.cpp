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

BroadcastSettings::BroadcastSettings(UserSettingsPointer pConfig)
    : m_pConfig(pConfig),
    m_profiles(),
    m_currentProfile(kDefaultProfile) {
    loadProfiles();
}

void BroadcastSettings::loadProfiles() {
    QDir profilesFolder(getProfilesFolder());
    if(!profilesFolder.exists()) {
        kLogger.warning()
                << "Profiles folder doesn't exist. Creating it.";
        profilesFolder.mkpath(profilesFolder.absolutePath());
    }

    QStringList nameFilters("*.bcp.xml");
    QFileInfoList files =
            profilesFolder.entryInfoList(nameFilters, QDir::Files, QDir::Name);

    if(files.count() > 0) {
        kLogger.warning() << "Found " << files.count() << " profiles.";
        // Load profiles from filesystem
        for(QFileInfo fileInfo : files) {
            BroadcastProfilePtr profile =
                    BroadcastProfile::loadFromFile(fileInfo.absoluteFilePath());

            if(profile)
               m_profiles.push_back(profile);
        }
    } else {
        kLogger.warning()
                << "No profiles found. Creating default profile.";

        // Create default profile
        BroadcastProfilePtr defaultProfile(
                    new BroadcastProfile(kDefaultProfile));
        // Upgrade from mixxx.cfg format to XML (if required)
        loadLegacySettings(defaultProfile);

        saveProfile(defaultProfile);

        m_profiles.push_back(defaultProfile);
        setCurrentProfile(defaultProfile);
    }
}

void BroadcastSettings::saveProfile(BroadcastProfilePtr profile) {
    if(!profile)
        return;

    profile->save(filenameForProfile(profile));
}

QString BroadcastSettings::filenameForProfile(const QString& profileName) {
    QString filename = profileName + QString(".bcp.xml");
    return QDir(getProfilesFolder()).absoluteFilePath(filename);
}

QString BroadcastSettings::filenameForProfile(BroadcastProfilePtr profile) {
    if(!profile)
        return QString();

    return filenameForProfile(profile->getProfileName());
}

QString BroadcastSettings::getProfilesFolder() {
    QString profilesPath(m_pConfig->getSettingsPath());
    profilesPath.append(QDir::separator() + QString(kProfilesSubfolder));
    return profilesPath;
}

void BroadcastSettings::setCurrentProfile(BroadcastProfilePtr profile) {
    if(!profile)
        return;

    QString profileName = profile->getProfileName();
    m_pConfig->setValue(ConfigKey(kConfigKey, kCurrentProfile), profileName);
}

BroadcastProfilePtr BroadcastSettings::getCurrentProfile() {
    QString currentProfile = m_pConfig->getValue(
                                 ConfigKey(kConfigKey, kCurrentProfile),
                                 kDefaultProfile);
    return getProfileByName(currentProfile);
}

BroadcastProfilePtr BroadcastSettings::getProfileByName(const QString& profileName) {
    for(BroadcastProfilePtr profile : m_profiles) {
        if(profile && profile->getProfileName() == profileName)
            return profile;
    }
    return BroadcastProfilePtr();
}

void BroadcastSettings::saveAll() {
    for(BroadcastProfilePtr profile : m_profiles) {
        if(profile)
            saveProfile(profile);
    }
}

void BroadcastSettings::deleteProfile(BroadcastProfilePtr profile) {
    if(!profile)
        return;

    QFileInfo xmlFile(filenameForProfile(profile));
    if(xmlFile.exists())
        QFile::remove(xmlFile.absolutePath());

    //m_profiles.removeAll(profile);
}
