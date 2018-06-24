#ifndef MISSINGTABLEMODEL_H
#define MISSINGTABLEMODEL_H

#include <QtSql>
#include <QItemDelegate>
#include <QModelIndex>
#include <QObject>

#include "library/basesqltablemodel.h"
#include "library/trackmodel.h"

class TrackCollection;

class MissingTableModel : public BaseSqlTableModel {
    Q_OBJECT
  public:
    MissingTableModel(QObject* parent, TrackCollection* pTrackCollection);
    ~MissingTableModel() final;

    void setTableModel(int id = -1);

    bool isColumnInternal(int column) final;
    void purgeTracks(const QModelIndexList& indices) final;
    Qt::ItemFlags flags(const QModelIndex &index) const final;
    CapabilitiesFlags getCapabilities() const final;

#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
    using QObject::parent;
#endif
};

#endif
