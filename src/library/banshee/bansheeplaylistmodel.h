#pragma once

#include <QHash>
#include <QtSql>

#include "library/trackmodel.h"
#include "library/trackcollection.h"
#include "library/dao/trackdao.h"
#include "library/banshee/bansheedbconnection.h"
#include "library/stardelegate.h"
#include "library/basesqltablemodel.h"

class BansheePlaylistModel final : public BaseSqlTableModel {
    Q_OBJECT
  public:
    BansheePlaylistModel(QObject* pParent, TrackCollectionManager* pTrackCollectionManager, BansheeDbConnection* pConnection);
    ~BansheePlaylistModel() final;

    void setTableModel(int playlistId);

    TrackPointer getTrack(const QModelIndex& index) const final;
    TrackId getTrackId(const QModelIndex& index) const final;

    QString getTrackLocation(const QModelIndex& index) const final;
    bool isColumnInternal(int column) final;

    Qt::ItemFlags flags(const QModelIndex &index) const final;
    CapabilitiesFlags getCapabilities() const final;

  private:
    TrackId doGetTrackId(const TrackPointer& pTrack) const final;

    QString getFieldString(const QModelIndex& index, const QString& fieldName) const;
    QVariant getFieldVariant(const QModelIndex& index, const QString& fieldName) const;
    void dropTempTable();

    BansheeDbConnection* m_pConnection;
    int m_playlistId;
    QString m_tempTableName;
};
