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

    void setTableModel(int id=-1);
    virtual TrackPointer getTrack(const QModelIndex& index) const;
    bool isColumnInternal(int column);
    bool isColumnHiddenByDefault(int column);
    Qt::ItemFlags flags(const QModelIndex &index) const;
    void setPlaylist(QString path_name);
    TrackModel::CapabilitiesFlags getCapabilities() const;

  private:
    QString m_playlistsTable;
    QString m_playlistTracksTable;
    QString m_trackSource;
};

#endif /* BASEEXTERNALPLAYLISTMODEL_H */
