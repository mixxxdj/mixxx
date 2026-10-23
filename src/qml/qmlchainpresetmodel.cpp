#include "qml/qmlchainpresetmodel.h"

#include <QModelIndex>

#include "effects/backends/effectmanifest.h"
#include "effects/effectsmanager.h"
#include "effects/presets/effectchainpreset.h"
#include "effects/presets/effectpreset.h"
#include "moc_qmlchainpresetmodel.cpp"

namespace mixxx {
namespace qml {
namespace {
const QHash<int, QByteArray> kRoleNames = {
        {Qt::DisplayRole, "display"},
        {Qt::ToolTipRole, "tooltip"},
        {QmlChainPresetModel::NameRole, "name"},
        {QmlChainPresetModel::ReadOnlyRole, "readOnly"},
        {QmlChainPresetModel::PresetDisplayRole, "presetDisplay"},
};
}

QmlChainPresetModel::QmlChainPresetModel(
        std::shared_ptr<EffectsManager> pEffectsManager,
        PresetType presetType,
        QObject* parent)
        : QAbstractListModel(parent),
          m_pEffectsManager(std::move(pEffectsManager)),
          m_pEffectChainPresetManager(m_pEffectsManager->getChainPresetManager()),
          m_presetType(presetType) {
    slotUpdated();
    if (m_presetType == PresetType::Quick) {
        connect(m_pEffectChainPresetManager.get(),
                &EffectChainPresetManager::quickEffectChainPresetListUpdated,
                this,
                &QmlChainPresetModel::slotUpdated);
    } else {
        connect(m_pEffectChainPresetManager.get(),
                &EffectChainPresetManager::effectChainPresetListUpdated,
                this,
                &QmlChainPresetModel::slotUpdated);
    }
}

void QmlChainPresetModel::slotUpdated() {
    beginResetModel();
    m_effectChainPresets = m_presetType == PresetType::Quick
            ? m_pEffectChainPresetManager->getQuickEffectPresetsSorted()
            : m_pEffectChainPresetManager->getPresetsSorted();
    endResetModel();
}

QVariant QmlChainPresetModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_effectChainPresets.size()) {
        return QVariant();
    }

    const EffectChainPresetPointer pPreset = m_effectChainPresets.at(index.row());
    switch (role) {
    case Qt::DisplayRole:
    case NameRole:
    case PresetDisplayRole:
        return pPreset->name();
    case ReadOnlyRole:
        return pPreset->isReadOnly();
    case Qt::ToolTipRole: {
        QStringList effectNames;
        for (const auto& pEffectPreset : pPreset->effectPresets()) {
            if (pEffectPreset->isEmpty()) {
                continue;
            }
            const EffectManifestPointer pManifest =
                    m_pEffectsManager->getBackendManager()->getManifest(pEffectPreset);
            if (pManifest) {
                effectNames.append(pManifest->name());
            }
        }
        QString tooltip = QStringLiteral("<b>%1</b>").arg(pPreset->name().toHtmlEscaped());
        if (effectNames.size() > 1) {
            tooltip.append(QStringLiteral("<br/>"));
            tooltip.append(effectNames.join(QStringLiteral("<br/>")));
        }
        return tooltip;
    }
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
