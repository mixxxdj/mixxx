#ifndef BANSHEEPLAYLISTMODEL_H
#define BANSHEEPLAYLISTMODEL_H

#include <QHash>
#include <QString>
#include <QVariant>
#include <QtCore>
#include <QtSql>

#include "library/banshee/bansheedbconnection.h"
#include "library/basesqltablemodel.h"
#include "library/dao/trackdao.h"
#include "library/stardelegate.h"
#include "library/trackcollection.h"
#include "library/trackmodel.h"
#include "track/track_decl.h"
#include "track/trackid.h"

class BansheeDbConnection;
class TrackCollectionManager;

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

#endif // BANSHEEPLAYLISTMODEL_H
