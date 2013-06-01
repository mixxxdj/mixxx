#ifndef BASEEXTERNALTRACKMODEL_H
#define BASEEXTERNALTRACKMODEL_H

#include <QModelIndex>
#include <QObject>
#include <QSqlDatabase>
#include <QString>

#include "library/trackmodel.h"
#include "library/basesqltablemodel.h"
#include "trackinfoobject.h"

class TrackCollection;

class BaseExternalTrackModel : public BaseSqlTableModel {
    Q_OBJECT
  public:
    BaseExternalTrackModel(QObject* parent, TrackCollection* pTrackCollection,
                           QString settingsNamespace,
                           QString trackTable,
                           QString trackSource);
    virtual ~BaseExternalTrackModel();

    void setTableModel(int id=-1);
    TrackModel::CapabilitiesFlags getCapabilities() const;
    TrackPointer getTrack(const QModelIndex& index) const;
    bool isColumnInternal(int column);
    bool isColumnHiddenByDefault(int column);
    Qt::ItemFlags flags(const QModelIndex &index) const;

  private:
    QString m_trackTable;
    QString m_trackSource;
};

#endif /* BASEEXTERNALTRACKMODEL_H */
