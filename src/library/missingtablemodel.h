#ifndef MISSINGTABLEMODEL_H
#define MISSINGTABLEMODEL_H

#include <QtSql>
#include <QItemDelegate>
#include <QModelIndex>
#include <QObject>

#include "trackmodel.h"
#include "library/basesqltablemodel.h"

class TrackCollection;

class MissingTableModel : public BaseSqlTableModel {
    Q_OBJECT
  public:
    MissingTableModel(QObject* parent, TrackCollection* pTrackCollection);
    virtual ~MissingTableModel();

    void setTableModel(int id = -1);
    bool isColumnInternal(int column);
    void purgeTracks(const QModelIndexList& indices);
    Qt::ItemFlags flags(const QModelIndex &index) const;
    TrackModel::CapabilitiesFlags getCapabilities() const;

  private:
    static const QString MISSINGFILTER;
};

#endif
