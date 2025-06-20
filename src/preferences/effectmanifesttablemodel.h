#pragma once

#include <QAbstractTableModel>
#include <QVariant>

#include "effects/defs.h"

/// EffectManifestTableModel represents a list of EffectManifests as a table
/// with a column for the BackendType and a column for the effect name.
/// EffectManifestTableModel is used by DlgPrefEffects to allow the user to drag
/// and drop between the lists of visible and hidden effects. It also allows the
/// user to rearrange the list of visible effects so they can choose which
/// effects can be loaded by controllers.
class EffectManifestTableModel : public QAbstractTableModel {
    Q_OBJECT
  public:
    EffectManifestTableModel(QObject* parent, EffectsBackendManagerPointer pBackendManager);
    ~EffectManifestTableModel() = default;

    const QList<EffectManifestPointer>& getList() const {
        return m_manifests;
    }
    void setList(const QList<EffectManifestPointer>& newList);

    // These functions are required for displaying data.
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant headerData(int section,
            Qt::Orientation orientation,
            int role = Qt::DisplayRole) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    // These functions are required for drag and drop.
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QMimeData* mimeData(const QModelIndexList& indexes) const override;
    // Set defaults so we can call it with mime data only for inserting
    // an effect at the end
    bool dropMimeData(
            const QMimeData* data,
            Qt::DropAction action = Qt::MoveAction,
            int row = -1,
            int column = -1,
            const QModelIndex& parent = QModelIndex()) override;
    QStringList mimeTypes() const override;
    Qt::DropActions supportedDropActions() const override;
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

  private:
    QList<EffectManifestPointer> m_manifests;
    EffectsBackendManagerPointer m_pBackendManager;
};
