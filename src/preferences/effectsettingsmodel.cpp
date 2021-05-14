#include "preferences/effectsettingsmodel.h"

#include "moc_effectsettingsmodel.cpp"

namespace {
const int kColumnEnabled = 0;
const int kColumnName = 1;
const int kColumnType = 2;
const int kNumberOfColumns = 3;
} // namespace

EffectSettingsModel::EffectSettingsModel() {
}

EffectSettingsModel::~EffectSettingsModel() {
}

void EffectSettingsModel::resetFromEffectManager(EffectsManager* pEffectsManager) {
    if (!pEffectsManager) {
        return;
    }

    if (!m_profiles.isEmpty()) {
        beginRemoveRows(QModelIndex(), 0, m_profiles.size()-1);
        endRemoveRows();
        m_profiles.clear();
    }

    for (const EffectManifestPointer& pManifest : pEffectsManager->getAvailableEffectManifests()) {
        const bool visibility = pEffectsManager->getEffectVisibility(pManifest);
        addProfileToModel(EffectProfilePtr(new EffectProfile(pManifest, visibility)));
    }
}

bool EffectSettingsModel::addProfileToModel(EffectProfilePtr profile) {
    if (!profile) {
        return false;
    }

    int position = m_profiles.size();
    beginInsertRows(QModelIndex(), position, position);

    m_profiles.push_back(EffectProfilePtr(profile));

    endInsertRows();
    return true;
}

void EffectSettingsModel::deleteProfileFromModel(EffectProfilePtr profile) {
    if (!profile) {
        return;
    }

    int position = m_profiles.indexOf(profile);
    if (position > -1) {
        beginRemoveRows(QModelIndex(), position, position);
        endRemoveRows();
    }
    m_profiles.removeAll(profile);
}

int EffectSettingsModel::rowCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    return m_profiles.size();
}

int EffectSettingsModel::columnCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    return kNumberOfColumns;
}

QVariant EffectSettingsModel::data(const QModelIndex& index, int role) const {
    int rowIndex = index.row();
    if (!index.isValid() || rowIndex >= m_profiles.size()) {
        return QVariant();
    }

    EffectProfilePtr profile = m_profiles.at(rowIndex);
    if (profile) {
        if (role == Qt::UserRole) {
            return profile->pManifest->id();
        }
        int column = index.column();
        if (column == kColumnEnabled) {
            if (role == Qt::CheckStateRole) {
                return (profile->bIsVisible ? Qt::Checked : Qt::Unchecked);
            } else if (role == Qt::TextAlignmentRole) {
                return Qt::AlignCenter;
            }
        } else if (column == kColumnName && role == Qt::DisplayRole) {
            return profile->pManifest->displayName();
        } else if (column == kColumnType && role == Qt::DisplayRole) {
            return profile->pManifest->translatedBackendName();
        }
    }

    return QVariant();
}

QVariant EffectSettingsModel::headerData(int section, Qt::Orientation orientation,
        int role) const {
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            if (section == kColumnEnabled) {
                return tr("Visible");
            } else if (section == kColumnName) {
                return tr("Name");
            } else if (section == kColumnType) {
                return tr("Type");
            }
        }
    }
    return QVariant();
}

Qt::ItemFlags EffectSettingsModel::flags(const QModelIndex& index) const {
    if (index.column() == kColumnEnabled) {
        return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;
    }

    if (index.column() == kColumnName) {
        return QAbstractItemModel::flags(index) | Qt::ItemIsSelectable;
    }

    if (index.column() == kColumnType) {
        return QAbstractItemModel::flags(index) | Qt::ItemIsSelectable;
    }

    return QAbstractItemModel::flags(index) | Qt::ItemIsEnabled;
}

bool EffectSettingsModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (index.isValid()) {
        EffectProfilePtr profile = m_profiles.at(index.row());
        if (profile) {
            if (index.column() == kColumnEnabled && role == Qt::CheckStateRole) {
                profile->bIsVisible = value.toBool();
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
