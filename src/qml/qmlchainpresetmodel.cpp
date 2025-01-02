#include "qml/qmlchainpresetmodel.h"

#include <QModelIndex>

#include "effects/backends/effectmanifest.h"
#include "effects/effectsmanager.h"
#include "effects/presets/effectchainpreset.h"
#include "moc_qmlchainpresetmodel.cpp"

namespace mixxx {
namespace qml {
namespace {
const QHash<int, QByteArray> kRoleNames = {
        {Qt::DisplayRole, "display"},
        {Qt::ToolTipRole, "tooltip"},
};
}

QmlChainPresetModel::QmlChainPresetModel(
        EffectChainPresetManagerPointer effectChainPresetManager,
        QObject* parent)
        : QAbstractListModel(parent),
          m_pEffectChainPresetManager(effectChainPresetManager) {
    slotUpdated();
    connect(m_pEffectChainPresetManager.get(),
            &EffectChainPresetManager::quickEffectChainPresetListUpdated,
            this,
            &QmlChainPresetModel::slotUpdated);
}

void QmlChainPresetModel::slotUpdated() {
    beginResetModel();
    m_effectChainPresets = m_pEffectChainPresetManager->getQuickEffectPresetsSorted();
    endResetModel();
}

QVariant QmlChainPresetModel::data(const QModelIndex& index, int role) const {
    if (index.row() >= m_effectChainPresets.size()) {
        return QVariant();
    }

    const EffectChainPresetPointer pPreset = m_effectChainPresets.at(index.row());
    switch (role) {
    case Qt::DisplayRole:
    case Qt::ToolTipRole:
        return pPreset->name();
    default:
        return QVariant();
    }
}

int QmlChainPresetModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }

    return m_effectChainPresets.size();
}

QHash<int, QByteArray> QmlChainPresetModel::roleNames() const {
    return kRoleNames;
}

QVariant QmlChainPresetModel::get(int row) const {
    QVariantMap dataMap;
    QModelIndex idx = index(row, 0);

    if (!idx.isValid()) {
        return dataMap;
    }

    for (auto it = kRoleNames.constBegin(); it != kRoleNames.constEnd(); it++) {
        dataMap.insert(it.value(), data(idx, it.key()));
    }
    return dataMap;
}

} // namespace qml
} // namespace mixxx
