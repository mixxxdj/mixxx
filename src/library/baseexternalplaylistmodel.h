#pragma once

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
    BaseExternalPlaylistModel(QObject* pParent, TrackCollectionManager* pTrackCollectionManager,
                              const char* settingsNamespace, const QString& playlistsTable,
                              const QString& playlistTracksTable, QSharedPointer<BaseTrackCache> trackSource);

    ~BaseExternalPlaylistModel() override;

    void setPlaylist(const QString& path_name);

    TrackPointer getTrack(const QModelIndex& index) const override;
    TrackId getTrackId(const QModelIndex& index) const override;
    bool isColumnInternal(int column) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    Capabilities getCapabilities() const override;

  private:
    TrackId doGetTrackId(const TrackPointer& pTrack) const override;

    QString m_playlistsTable;
    QString m_playlistTracksTable;
    QSharedPointer<BaseTrackCache> m_trackSource;
};
