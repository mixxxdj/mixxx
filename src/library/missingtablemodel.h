#pragma once

#include <QItemDelegate>
#include <QModelIndex>
#include <QObject>
#include <QtSql>

#include "library/basesqltablemodel.h"
#include "library/trackmodel.h"

class MissingTableModel : public BaseSqlTableModel {
    Q_OBJECT
  public:
    MissingTableModel(QObject* parent, TrackCollectionManager* pTrackCollectionManager);
    ~MissingTableModel() final;

    void setTableModel(int id = -1);

    bool isColumnInternal(int column) final;
    void purgeTracks(const QModelIndexList& indices) final;
    Qt::ItemFlags flags(const QModelIndex &index) const final;
    CapabilitiesFlags getCapabilities() const final;
};
