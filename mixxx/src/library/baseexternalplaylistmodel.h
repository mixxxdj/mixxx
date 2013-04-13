#ifndef BASEEXTERNALPLAYLISTMODEL_H
#define BASEEXTERNALPLAYLISTMODEL_H

#include <QtSql>
#include <QItemDelegate>
#include <QtCore>

#include "library/trackmodel.h"
#include "library/basesqltablemodel.h"
#include "library/librarytablemodel.h"
#include "library/dao/playlistdao.h"
#include "library/dao/trackdao.h"

class BaseExternalPlaylistModel : public BaseSqlTableModel {
    Q_OBJECT
  public:
    BaseExternalPlaylistModel(QObject* pParent, TrackCollection* pTrackCollection,
                              QString settingsNamespace, QString playlistsTable,
                              QString playlistTracksTable, QString trackSource);
    virtual ~BaseExternalPlaylistModel();

    virtual TrackPointer getTrack(const QModelIndex& index) const;
    virtual void search(const QString& searchText);
    virtual bool isColumnInternal(int column);
    virtual bool isColumnHiddenByDefault(int column);
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual void setPlaylist(QString path_name);
    virtual TrackModel::CapabilitiesFlags getCapabilities() const;

  private slots:
    void slotSearch(const QString& searchText);

  signals:
    void doSearch(const QString& searchText);

  private:
    QString m_playlistsTable;
    QString m_playlistTracksTable;
    QString m_trackSource;

    TrackCollection* m_pTrackCollection;
    QSqlDatabase& m_database;
};

#endif /* BASEEXTERNALPLAYLISTMODEL_H */
