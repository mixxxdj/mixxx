#pragma once

#include <QStringListModel>

#include "effects/defs.h"

/// EffectChainPresetListModel is used to represent the list of effect chain
/// presets for the standard effect chains and QuickEffect chains in
/// DlgPrefEffects. A custom QStringListModel subclass is required so items can
/// be copied between the two lists without creating duplicates when dragging
/// and dropping within the lists.
class EffectChainPresetListModel : public QStringListModel {
    Q_OBJECT
  public:
    EffectChainPresetListModel(QObject* parent, EffectChainPresetManagerPointer pPresetManager);

    QMimeData* mimeData(const QModelIndexList& indexes) const override;
    bool dropMimeData(
            const QMimeData* data,
            Qt::DropAction action,
            int row,
            int column,
            const QModelIndex& parent) override;
    QStringList mimeTypes() const override;
    Qt::DropActions supportedDropActions() const override;

    // required to make items not editable
    Qt::ItemFlags flags(const QModelIndex& index) const override;

  private:
    EffectChainPresetManagerPointer m_pPresetManager;
};
