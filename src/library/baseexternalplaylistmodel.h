#ifndef BASEEXTERNALPLAYLISTMODEL_H
#define BASEEXTERNALPLAYLISTMODEL_H

#include <QItemDelegate>
#include <QModelIndex>
#include <QObject>
#include <QSharedPointer>
#include <QString>
#include <QtCore>
#include <QtSql>

#include "library/basesqltablemodel.h"
#include "library/dao/playlistdao.h"
#include "library/dao/trackdao.h"
#include "library/librarytablemodel.h"
#include "library/trackmodel.h"
#include "track/track_decl.h"
#include "track/trackid.h"

class BaseTrackCache;
class TrackCollectionManager;
template<class T>
class QSharedPointer;

class BaseExternalPlaylistModel : public BaseSqlTableModel {
    Q_OBJECT
  public:
    BaseExternalPlaylistModel(QObject* pParent, TrackCollectionManager* pTrackCollectionManager,
                              const char* settingsNamespace, const QString& playlistsTable,
                              const QString& playlistTracksTable, QSharedPointer<BaseTrackCache> trackSource);

    ~BaseExternalPlaylistModel() override;

    void setPlaylist(const QString& path_name);

    TrackPointer getTrack(const QModelIndex& index) const override;
    TrackId getTrackId(const QModelIndex& index) const override;
    bool isColumnInternal(int column) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    CapabilitiesFlags getCapabilities() const override;

  private:
    TrackId doGetTrackId(const TrackPointer& pTrack) const override;

    QString m_playlistsTable;
    QString m_playlistTracksTable;
    QSharedPointer<BaseTrackCache> m_trackSource;
};

#endif /* BASEEXTERNALPLAYLISTMODEL_H */
