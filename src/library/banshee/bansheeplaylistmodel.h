#pragma once

#include "library/trackmodel.h"
#include "library/basesqltablemodel.h"

class BansheeDbConnection;

class BansheePlaylistModel final : public BaseSqlTableModel {
    Q_OBJECT
  public:
    BansheePlaylistModel(QObject* pParent, TrackCollectionManager* pTrackCollectionManager, BansheeDbConnection* pConnection);
    ~BansheePlaylistModel() final;

    void selectPlaylist(int playlistId);

    TrackPointer getTrack(const QModelIndex& index) const final;
    TrackId getTrackId(const QModelIndex& index) const final;
    QUrl getTrackUrl(const QModelIndex& index) const final;

    QString getTrackLocation(const QModelIndex& index) const final;
    bool isColumnInternal(int column) final;

    Qt::ItemFlags flags(const QModelIndex &index) const final;
    Capabilities getCapabilities() const final;

  private:
    TrackId doGetTrackId(const TrackPointer& pTrack) const final;
    void dropTempTable();

    BansheeDbConnection* m_pConnection;
    int m_playlistId;
    QString m_tempTableName;
};
