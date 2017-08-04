#include <QDir>
#include <QStringList>
#include <QFileInfoList>
#include <QFileInfo>

#include "broadcast/defs_broadcast.h"
#include "defs_urls.h"
#include "preferences/broadcastsettings.h"
#include "preferences/pushbuttondelegate.h"
#include "util/logger.h"
#include "util/memory.h"

namespace {
const char* kProfilesSubfolder = "broadcast_profiles";
const char* kDefaultProfile = "Profile 1"; // Must be used only when initializing profiles
const mixxx::Logger kLogger("BroadcastSettings");
const int kColumnEnabled = 0;
const int kColumnName = 1;
const int kColumnStatus = 2;
const int kColumnRemove = 3;
} // anonymous namespace

BroadcastSettings::BroadcastSettings(
        UserSettingsPointer pConfig, QObject* parent)
    : QAbstractTableModel(parent),
      m_pConfig(pConfig),
      m_profiles() {
    loadProfiles();
}

void BroadcastSettings::loadProfiles() {
    QDir profilesFolder(getProfilesFolder());
    if(!profilesFolder.exists()) {
        kLogger.info() << "Profiles folder doesn't exist. Creating it.";
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

    if(files.size() > 0) {
        kLogger.info() << "Found " << files.size() << " profile(s)";

        // Load profiles from filesystem
        for(QFileInfo fileInfo : files) {
            BroadcastProfilePtr profile =
                    BroadcastProfile::loadFromFile(fileInfo.absoluteFilePath());

            if(profile)
                addProfile(profile);
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

bool BroadcastSettings::addProfile(const BroadcastProfilePtr& profile) {
    if(!profile)
        return false;

    int position = m_profiles.size();
    beginInsertRows(QModelIndex(), position, position);

    // It is best to avoid using QSharedPointer::data(), especially when
    // passing it to another function, as it puts the associated pointer
    // at risk of being manually deleted.
    // However it's fine with Qt's connect because it can be trusted that
    // it won't delete the pointer.
    connect(profile.data(), SIGNAL(profileNameChanged(QString, QString)),
            this, SLOT(onProfileNameChanged(QString,QString)));
    m_profiles.insert(profile->getProfileName(), BroadcastProfilePtr(profile));

    endInsertRows();

    emit profileAdded(profile);
    return true;
}

BroadcastProfilePtr BroadcastSettings::createProfile(const QString& profileName) {
    QFileInfo xmlFile(filePathForProfile(profileName));

    if(!xmlFile.exists()) {
        BroadcastProfilePtr profile(new BroadcastProfile(profileName));
        saveProfile(profile);
        addProfile(profile);
        return profile;
    }
    return BroadcastProfilePtr(nullptr);
}

bool BroadcastSettings::saveProfile(const BroadcastProfilePtr& profile) {
    if(!profile)
        return false;

    return profile->save(filePathForProfile(profile));
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

bool BroadcastSettings::deleteFileForProfile(const BroadcastProfilePtr& profile) {
    if(!profile)
        return false;

    return deleteFileForProfile(profile->getProfileName());
}

bool BroadcastSettings::deleteFileForProfile(const QString& profileName) {
    QFileInfo xmlFile(filePathForProfile(profileName));
    if(xmlFile.exists()) {
        return QFile::remove(xmlFile.absoluteFilePath());
    }
    return false;
}

QString BroadcastSettings::getProfilesFolder() {
    QString profilesPath(m_pConfig->getSettingsPath());
    profilesPath.append(QDir::separator() + QString(kProfilesSubfolder));
    return profilesPath;
}

BroadcastProfilePtr BroadcastSettings::getProfileByName(
        const QString& profileName) {
    return m_profiles.value(profileName, BroadcastProfilePtr(nullptr));
}

void BroadcastSettings::saveAll() {
    for(auto kv : m_profiles.values()) {
        saveProfile(kv);
    }
    emit profilesChanged();
}

void BroadcastSettings::deleteProfile(const BroadcastProfilePtr& profile) {
    if(!profile)
        return;

    deleteFileForProfile(profile);

    int position = m_profiles.keys().indexOf(profile->getProfileName());
    if(position > -1) {
        beginRemoveRows(QModelIndex(), position, position);
        endRemoveRows();
    }
    m_profiles.remove(profile->getProfileName());

    emit profileRemoved(profile);
}

void BroadcastSettings::onProfileNameChanged(QString oldName, QString newName) {
    if(!m_profiles.contains(oldName))
        return;

    BroadcastProfilePtr oldItem = m_profiles.take(oldName);
    if(oldItem) {
        m_profiles.insert(newName, oldItem);
        emit profileRenamed(oldName, oldItem);

        deleteFileForProfile(oldName);

    }
}

int BroadcastSettings::rowCount(const QModelIndex& parent) const {
    return m_profiles.size();
}

int BroadcastSettings::columnCount(const QModelIndex& parent) const {
    return 4;
}

QVariant BroadcastSettings::data(const QModelIndex& index, int role) const {
    int rowIndex = index.row();
    if(!index.isValid() || rowIndex >= m_profiles.size())
        return QVariant();

    BroadcastProfilePtr profile = m_profiles.values().at(rowIndex);
    if(profile) {
        int column = index.column();
        if(column == kColumnEnabled && role == Qt::CheckStateRole) {
            return (profile->getEnabled() == true ? Qt::Checked : Qt::Unchecked);
        } else if(column == kColumnName
        		&& (role == Qt::DisplayRole || role == Qt::EditRole)) {
            return profile->getProfileName();
        } else if(column == kColumnStatus && role == Qt::DisplayRole) {
            return "placeholder";
        } else if(column == kColumnRemove && role == Qt::DisplayRole) {
            return tr("Double-click to remove");
        }
    }

    return QVariant();
}

QVariant BroadcastSettings::headerData(int section, Qt::Orientation orientation,
        int role) const {
    if(orientation == Qt::Horizontal) {
        if(role == Qt::DisplayRole) {
            if(section == kColumnEnabled) {
                return tr("Enabled");
            } else if(section == kColumnName) {
                return tr("Name");
            } else if(section == kColumnStatus) {
                return tr("Status");
            } else if(section == kColumnRemove) {
                return QString("");
            }
        }
    }
    return QVariant();
}

Qt::ItemFlags BroadcastSettings::flags(const QModelIndex& index) const {
    if(index.column() == kColumnEnabled)
        return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;

    if(index.column() == kColumnName)
        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;

    if(index.column() == kColumnRemove)
        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;

    return Qt::ItemIsEnabled;
}

bool BroadcastSettings::setData(const QModelIndex& index, const QVariant& value, int role) {
    if(index.isValid()) {
        BroadcastProfilePtr profile = profileAt(index.row());
        if(profile) {
            if(index.column() == kColumnEnabled && role == Qt::CheckStateRole) {
                profile->setEnabled(value.toBool());
            }
            if(index.column() == kColumnName && role == Qt::EditRole) {
                QString newName = value.toString();
                newName = newName.trimmed();

                if(!newName.isNull() && !newName.isEmpty())
                    profile->setProfileName(newName);
            }
        }
    }
    return true;
}

QAbstractItemDelegate* BroadcastSettings::delegateForColumn(const int i, QObject* parent) {
    if(i == kColumnRemove) {
        return new PushButtonDelegate(parent);
    }
    return nullptr;
}

BroadcastProfilePtr BroadcastSettings::profileAt(int index) {
    return m_profiles.values().value(index, BroadcastProfilePtr(nullptr));
}

QList<BroadcastProfilePtr> BroadcastSettings::profiles() {
    return m_profiles.values();
}
