// broadcastsettingsmodel.cpp
// Created on August 7th by Stéphane Lepin (Palakis)

#include <preferences/broadcastsettingsmodel.h>

#include <preferences/broadcastsettings.h>

namespace {
const int kColumnEnabled = 0;
const int kColumnName = 1;
const int kColumnStatus = 2;
}

BroadcastSettingsModel::BroadcastSettingsModel() {
}

void BroadcastSettingsModel::resetFromSettings(BroadcastSettingsPointer pSettings) {
    if (!pSettings) {
        return;
    }

    if (!m_profiles.isEmpty()) {
        beginRemoveRows(QModelIndex(), 0, m_profiles.size()-1);
        endRemoveRows();
        m_profiles.clear();
    }

    const QList<BroadcastProfilePtr> profiles = pSettings->profiles();
    for (BroadcastProfilePtr profile : profiles) {
        BroadcastProfilePtr copy = profile->valuesCopy();
        copy->setConnectionStatus(profile->connectionStatus());
        connect(profile.data(),
                &BroadcastProfile::statusChanged,
                copy.data(),
                &BroadcastProfile::relayStatus);
        connect(profile.data(),
                &BroadcastProfile::connectionStatusChanged,
                copy.data(),
                &BroadcastProfile::relayConnectionStatus);
        addProfileToModel(copy);
    }
}

bool BroadcastSettingsModel::addProfileToModel(BroadcastProfilePtr profile) {
    if (!profile)
        return false;

    int position = m_profiles.size();
    beginInsertRows(QModelIndex(), position, position);

    // It is best to avoid using QSharedPointer::data(), especially when
    // passing it to another function, as it puts the associated pointer
    // at risk of being manually deleted.
    // However it's fine with Qt's connect because it can be trusted that
    // it won't delete the pointer.
    connect(profile.data(),
            &BroadcastProfile::profileNameChanged,
            this,
            &BroadcastSettingsModel::onProfileNameChanged);
    connect(profile.data(),
            &BroadcastProfile::connectionStatusChanged,
            this,
            &BroadcastSettingsModel::onConnectionStatusChanged);
    m_profiles.insert(profile->getProfileName(), BroadcastProfilePtr(profile));

    endInsertRows();
    return true;
}

void BroadcastSettingsModel::deleteProfileFromModel(BroadcastProfilePtr profile) {
    if (!profile)
        return;

    QString name = profile->getProfileName();
    int position = 0;
    for (auto it = m_profiles.constBegin(); it != m_profiles.constEnd(); ++it, ++position) {
        if (it.key() == name) {
            beginRemoveRows(QModelIndex(), position, position);
            endRemoveRows();
        }
    }
    m_profiles.remove(profile->getProfileName());
}

BroadcastProfilePtr BroadcastSettingsModel::getProfileByName(
        const QString& profileName) {
    return m_profiles.value(profileName, BroadcastProfilePtr());
}

int BroadcastSettingsModel::rowCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    return m_profiles.size();
}

int BroadcastSettingsModel::columnCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    return 3;
}

QVariant BroadcastSettingsModel::data(const QModelIndex& index, int role) const {
    int rowIndex = index.row();
    if (!index.isValid() || rowIndex >= m_profiles.size())
        return QVariant();

    auto it = m_profiles.constBegin() + rowIndex;
    if (it != m_profiles.constEnd()) {
        BroadcastProfilePtr profile = it.value();
        int column = index.column();
        if (column == kColumnEnabled) {
            if (role == Qt::CheckStateRole) {
                return (profile->getEnabled() == true ? Qt::Checked : Qt::Unchecked);
            }
            else if (role == Qt::TextAlignmentRole) {
                return Qt::AlignCenter;
            }
        }
        else if (column == kColumnName && role == Qt::DisplayRole) {
            return profile->getProfileName();
        }
        else if (column == kColumnStatus) {
            if (role == Qt::DisplayRole) {
                return connectionStatusString(profile);
            }
            else if (role == Qt::BackgroundRole) {
                return QBrush(connectionStatusColor(profile));
            }
            else if (role == Qt::TextAlignmentRole) {
                return Qt::AlignCenter;
            }
        }
    }

    return QVariant();
}

QVariant BroadcastSettingsModel::headerData(int section, Qt::Orientation orientation,
        int role) const {
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            if (section == kColumnEnabled) {
                return tr("Enabled");
            } else if (section == kColumnName) {
                return tr("Name");
            } else if (section == kColumnStatus) {
                return tr("Status");
            }
        }
    }
    return QVariant();
}

Qt::ItemFlags BroadcastSettingsModel::flags(const QModelIndex& index) const {
    if (index.column() == kColumnEnabled)
        return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;

    if (index.column() == kColumnName)
        return QAbstractItemModel::flags(index) | Qt::ItemIsSelectable;

    return QAbstractItemModel::flags(index) | Qt::ItemIsEnabled;
}

bool BroadcastSettingsModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (index.isValid()) {
        auto it = m_profiles.constBegin() + index.row();
        if (it != m_profiles.constEnd()) {
            BroadcastProfilePtr profile = it.value();
            if (index.column() == kColumnEnabled && role == Qt::CheckStateRole) {
                profile->setEnabled(value.toBool());
            }
            if (index.column() == kColumnName && role == Qt::EditRole) {
                QString newName = value.toString();
                newName = newName.trimmed();

                if (!newName.isNull() && !newName.isEmpty())
                    profile->setProfileName(newName);
            }
        }
    }
    return true;
}

QAbstractItemDelegate* BroadcastSettingsModel::delegateForColumn(const int i, QObject* parent) {
    Q_UNUSED(i);
    Q_UNUSED(parent);
    return nullptr;
}

QString BroadcastSettingsModel::connectionStatusString(BroadcastProfilePtr profile) {
    int status = profile->connectionStatus();
    switch(status) {
        case BroadcastProfile::STATUS_UNCONNECTED:
            return tr("Disconnected");
        case BroadcastProfile::STATUS_CONNECTING:
            return tr("Connecting...");
        case BroadcastProfile::STATUS_CONNECTED:
            return tr("Connected");
        case BroadcastProfile::STATUS_FAILURE:
            return tr("Failed");

        default:
            return tr("Unknown");
    }
}

QColor BroadcastSettingsModel::connectionStatusColor(BroadcastProfilePtr profile) {
    // Manual colors below were picked using Google's color picker (query: colorpicker)
    //
    int status = profile->connectionStatus();
        switch(status) {
            case BroadcastProfile::STATUS_UNCONNECTED:
                return Qt::white;
            case BroadcastProfile::STATUS_CONNECTING:
                return QColor(25, 224, 255); // turquoise blue
            case BroadcastProfile::STATUS_CONNECTED:
                return QColor(96, 255, 81); // warm green
            case BroadcastProfile::STATUS_FAILURE:
                return QColor(255, 228, 56); // toned-down yellow

            default:
                return Qt::white;
        }
}

void BroadcastSettingsModel::onProfileNameChanged(QString oldName, QString newName) {
    if (!m_profiles.contains(oldName))
        return;

    BroadcastProfilePtr profile = m_profiles.take(oldName);
    if (profile) {
        m_profiles.insert(newName, profile);
    }

    // Refresh the whole name column
    QModelIndex start = this->index(0, kColumnName);
    QModelIndex end = this->index(this->rowCount()-1, kColumnName);
    emit dataChanged(start, end);
}

void BroadcastSettingsModel::onConnectionStatusChanged(int newStatus) {
    Q_UNUSED(newStatus);
    // Refresh the whole status column
    QModelIndex start = this->index(0, kColumnStatus);
    QModelIndex end = this->index(this->rowCount()-1, kColumnStatus);
    emit dataChanged(start, end);
}
