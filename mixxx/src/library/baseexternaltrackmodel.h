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

    TrackModel::CapabilitiesFlags getCapabilities() const;
    virtual TrackPointer getTrack(const QModelIndex& index) const;
    virtual void search(const QString& searchText);
    virtual bool isColumnInternal(int column);
    virtual bool isColumnHiddenByDefault(int column);
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

  private slots:
    void slotSearch(const QString& searchText);

  signals:
    void doSearch(const QString& searchText);

  private:
    TrackCollection* m_pTrackCollection;
    QSqlDatabase& m_database;
};

#endif /* BASEEXTERNALTRACKMODEL_H */
