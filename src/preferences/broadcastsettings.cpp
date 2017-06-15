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
const QString kDefaultProfile = QObject::tr("Untitled Profile");
const mixxx::Logger kLogger("BroadcastSettings");
} // anonymous namespace

BroadcastSettings::BroadcastSettings(UserSettingsPointer pConfig,
                                     QObject *parent)
    : QAbstractListModel(parent),
    m_pConfig(pConfig),
    m_profiles(),
    m_currentProfile(kDefaultProfile) {
    setHeaderData(0, Qt::Horizontal, QObject::tr("Enabled"), Qt::EditRole);
    setHeaderData(1, Qt::Horizontal, QObject::tr("Name"), Qt::EditRole);
    setHeaderData(2, Qt::Horizontal, QObject::tr("Edit"), Qt::DisplayRole);
    setHeaderData(3, Qt::Horizontal, QObject::tr("Remove"), Qt::DisplayRole);

    loadProfiles();
}

BroadcastSettings::~BroadcastSettings() {
    for(BroadcastProfile* profile : m_profiles) {
        delete profile;
    }
}

void BroadcastSettings::loadProfiles() {
    QDir profilesFolder(getProfilesFolder());
    if(!profilesFolder.exists()) {
        kLogger.warning()
                << "Profiles folder doesn't exist. Creating it.";

        // TODO(Palakis, June 9th 2017):
        // Is there a better way to do this?
        profilesFolder.mkpath(profilesFolder.absolutePath());
    }

    QStringList nameFilters("*.bcp.xml");
    QFileInfoList files =
            profilesFolder.entryInfoList(nameFilters, QDir::Files, QDir::Name);

    if(files.count() > 0) {
        kLogger.warning() << "Found " << files.count() << " profiles.";
        // Load profiles from filesystem
        for(QFileInfo fileInfo : files) {
            BroadcastProfile* profile =
                    BroadcastProfile::loadFromFile(fileInfo.absoluteFilePath());

            if(profile)
               addProfile(profile);
        }
    } else {
        kLogger.warning()
                << "No profiles found. Creating default profile.";

        // Create default profile
        BroadcastProfile* defaultProfile = newProfile();
        setCurrentProfile(defaultProfile);
    }
}

BroadcastProfile* BroadcastSettings::newProfile() {
    QFileInfo xmlFile(filenameForProfile(kDefaultProfile));

    if(!xmlFile.exists()) {
        BroadcastProfile* profile = new BroadcastProfile(kDefaultProfile);
        saveProfile(profile);
        addProfile(profile);
        return profile;
    }
    return nullptr;
}

void BroadcastSettings::saveProfile(BroadcastProfile* profile) {
    if(!profile)
        return;

    profile->save(filenameForProfile(profile));
}

QString BroadcastSettings::filenameForProfile(const QString& profileName) {
    QString filename = profileName + QString(".bcp.xml");
    return QDir(getProfilesFolder()).absoluteFilePath(filename);
}

QString BroadcastSettings::filenameForProfile(BroadcastProfile* profile) {
    if(!profile)
        return QString();

    return filenameForProfile(profile->getProfileName());
}

QString BroadcastSettings::getProfilesFolder() {
    QString profilesPath(m_pConfig->getSettingsPath());
    profilesPath.append(QDir::separator() + QString(kProfilesSubfolder));
    return profilesPath;
}

bool BroadcastSettings::addProfile(BroadcastProfile *profile) {
    if(profile) {
        int position = m_profiles.size();

        beginInsertRows(QModelIndex(), position, position);
        m_profiles.push_back(profile);
        endInsertRows();

        return true;
    }
    return false;
}

void BroadcastSettings::setCurrentProfile(BroadcastProfile *profile) {
    if(!profile)
        return;

    QString profileName = profile->getProfileName();
    m_pConfig->setValue(ConfigKey(kConfigKey, kCurrentProfile), profileName);
}

BroadcastProfile* BroadcastSettings::getCurrentProfile() {
    QString currentProfile = m_pConfig->getValue(
                                 ConfigKey(kConfigKey, kCurrentProfile),
                                 kDefaultProfile);
    return getProfileByName(currentProfile);
}

BroadcastProfile* BroadcastSettings::getProfileByName(const QString& profileName) {
    for(BroadcastProfile* profile : m_profiles) {
        if(profile && profile->getProfileName() == profileName)
            return profile;
    }
    return nullptr;
}

void BroadcastSettings::saveAll() {
    for(BroadcastProfile* profile : m_profiles) {
        if(profile)
            saveProfile(profile);
    }
}

void BroadcastSettings::renameProfile(BroadcastProfile* profile,
                                      const QString& newName) {

}

void BroadcastSettings::deleteProfile(BroadcastProfile* profile) {
    if(!profile)
        return;

    QFileInfo xmlFile(filenameForProfile(profile));
    if(xmlFile.exists())
        QFile::remove(xmlFile.absolutePath());

    m_profiles.removeAll(profile);
}

int BroadcastSettings::rowCount(const QModelIndex &parent) const {
    return m_profiles.size();
}

QVariant BroadcastSettings::data(const QModelIndex &index, int role) const {
    int rowIndex = index.row();

    if(!index.isValid()
       || rowIndex >= m_profiles.size())
        return QVariant();

    if(role == Qt::DisplayRole)
        return m_profiles.at(rowIndex)->getProfileName();
    else
        return QVariant();
}

/*QVariant BroadcastSettings::headerData(int section, Qt::Orientation orientation,
                    int role = Qt::DisplayRole) {

}*/
