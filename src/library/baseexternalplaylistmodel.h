#ifndef BASEEXTERNALPLAYLISTMODEL_H
#define BASEEXTERNALPLAYLISTMODEL_H

#include <QtSql>
#include <QItemDelegate>
#include <QString>
#include <QObject>
#include <QModelIndex>

#include "library/trackmodel.h"
#include "library/basesqltablemodel.h"
#include "library/librarytablemodel.h"
#include "library/dao/playlistdao.h"
#include "library/dao/trackdao.h"

class BaseExternalPlaylistModel : public BaseSqlTableModel {
    Q_OBJECT
  public:
    BaseExternalPlaylistModel(QObject* pParent, TrackCollection* pTrackCollection,
                              const char* settingsNamespace, const QString& playlistsTable,
                              const QString& playlistTracksTable, QSharedPointer<BaseTrackCache> trackSource);

    virtual ~BaseExternalPlaylistModel();

    virtual TrackPointer getTrack(const QModelIndex& index) const;
    virtual bool isColumnInternal(int column);
    Qt::ItemFlags flags(const QModelIndex &index) const;
    void setPlaylist(QString path_name);
    virtual TrackModel::CapabilitiesFlags getCapabilities() const;

  private:
    QString m_playlistsTable;
    QString m_playlistTracksTable;
    QSharedPointer<BaseTrackCache> m_trackSource;
};

#endif /* BASEEXTERNALPLAYLISTMODEL_H */
