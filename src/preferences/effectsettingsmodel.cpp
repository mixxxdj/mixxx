#include <preferences/effectsettingsmodel.h>

// #include <preferences/effectsettings.h>
#include "effects/effectmanifest.h"

namespace {
const int kColumnEnabled = 0;
const int kColumnName = 1;
}

EffectSettingsModel::EffectSettingsModel() {
}

EffectSettingsModel::~EffectSettingsModel() {
}

void EffectSettingsModel::resetFromEffectManager(EffectsManager* pEffectsManager) {
    if(!pEffectsManager) {
        return;
    }

    beginRemoveRows(QModelIndex(), 0, m_profiles.size()-1);
    endRemoveRows();
    m_profiles.clear();

    for(EffectManifestPointer pManifest : pEffectsManager->getAvailableEffectManifests()) {
        // qDebug() << "Manifest: " << pManifest->id();
        addProfileToModel(EffectProfilePtr(new EffectProfile(*pManifest)));
    }
}

bool EffectSettingsModel::addProfileToModel(EffectProfilePtr profile) {
    if(!profile)
        return false;

    int position = m_profiles.size();
    beginInsertRows(QModelIndex(), position, position);

    m_profiles.insert(profile->getEffectId(), EffectProfilePtr(profile));

    endInsertRows();
    return true;
}

void EffectSettingsModel::deleteProfileFromModel(EffectProfilePtr profile) {
    if(!profile)
        return;

    int position = m_profiles.keys().indexOf(profile->getEffectId());
    if(position > -1) {
        beginRemoveRows(QModelIndex(), position, position);
        endRemoveRows();
    }
    m_profiles.remove(profile->getEffectId());
}

int EffectSettingsModel::rowCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    return m_profiles.size();
}

int EffectSettingsModel::columnCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    return 2;
}

QVariant EffectSettingsModel::data(const QModelIndex& index, int role) const {
    int rowIndex = index.row();
    if (!index.isValid() || rowIndex >= m_profiles.size())
        return QVariant();

    EffectProfilePtr profile = m_profiles.values().at(rowIndex);
    if (profile) {
        if (role == Qt::UserRole)
            return profile->getEffectId();
        int column = index.column();
        if (column == kColumnEnabled) {
            if (role == Qt::CheckStateRole) {
                return (profile->isVisible() == true ? Qt::Checked : Qt::Unchecked);
            }
            else if (role == Qt::TextAlignmentRole) {
                return Qt::AlignCenter;
            }
        }
        else if (column == kColumnName && role == Qt::DisplayRole) {
            return profile->getDisplayName();
        }
    }

    return QVariant();
}

QVariant EffectSettingsModel::headerData(int section, Qt::Orientation orientation,
        int role) const {
    if(orientation == Qt::Horizontal) {
        if(role == Qt::DisplayRole) {
            if(section == kColumnEnabled) {
                return tr("Visible");
            } else if(section == kColumnName) {
                return tr("Name");
            }
        }
    }
    return QVariant();
}

Qt::ItemFlags EffectSettingsModel::flags(const QModelIndex& index) const {
    if(index.column() == kColumnEnabled)
        return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;

    if(index.column() == kColumnName)
        return QAbstractItemModel::flags(index) | Qt::ItemIsSelectable;

    return QAbstractItemModel::flags(index) | Qt::ItemIsEnabled;
}

bool EffectSettingsModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if(index.isValid()) {
        EffectProfilePtr profile = m_profiles.values().at(index.row());
        if(profile) {
            if(index.column() == kColumnEnabled && role == Qt::CheckStateRole) {
                profile->setVisibility(value.toBool());
            }
        }
    }
    return true;
}

QAbstractItemDelegate* EffectSettingsModel::delegateForColumn(const int i, QObject* parent) {
    Q_UNUSED(i);
    Q_UNUSED(parent);
    return nullptr;
}

bool EffectSettingsModel::isEmpty() const {
    return m_profiles.isEmpty();
}
