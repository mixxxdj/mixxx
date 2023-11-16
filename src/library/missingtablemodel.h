#pragma once

#include <QItemDelegate>
#include <QObject>

#include "library/basesqltablemodel.h"
#include "library/trackmodel.h"

class QModelIndex;
typedef QList<QModelIndex> QModelIndexList;

class MissingTableModel final : public BaseSqlTableModel {
    Q_OBJECT
  public:
    MissingTableModel(QObject* parent, TrackCollectionManager* pTrackCollectionManager);
    ~MissingTableModel() final;

    void setTableModel(int id = -1);

    bool isColumnInternal(int column) final;
    void purgeTracks(const QModelIndexList& indices) final;
    Qt::ItemFlags flags(const QModelIndex& index) const final;
    Capabilities getCapabilities() const final;

    QString modelKey(bool noSearch) const override;
};
