#ifndef BANSHEEPLAYLISTMODEL_H
#define BANSHEEPLAYLISTMODEL_H

#include <QHash>
#include <QtSql>

#include "library/trackmodel.h"
#include "library/trackcollection.h"
#include "library/dao/trackdao.h"
#include "library/banshee/bansheedbconnection.h"
#include "library/stardelegate.h"
#include "library/basesqltablemodel.h"

class BansheePlaylistModel : public BaseSqlTableModel {
    Q_OBJECT
  public:
    BansheePlaylistModel(QObject* pParent, TrackCollection* pTrackCollection, BansheeDbConnection* pConnection);
    virtual ~BansheePlaylistModel();

    void setTableModel(int playlistId);

    virtual TrackPointer getTrack(const QModelIndex& index) const;
    virtual QString getTrackLocation(const QModelIndex& index) const;
    virtual bool isColumnInternal(int column);

    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    TrackModel::CapabilitiesFlags getCapabilities() const;

    virtual bool setData(const QModelIndex& index, const QVariant& value, int role=Qt::EditRole);


  protected:
    // Use this if you want a model that is read-only.
    virtual Qt::ItemFlags readOnlyFlags(const QModelIndex &index) const;
    // Use this if you want a model that can be changed
    virtual Qt::ItemFlags readWriteFlags(const QModelIndex &index) const;

  private slots:
    virtual void tracksChanged(QSet<int> trackIds);
    virtual void trackLoaded(QString group, TrackPointer pTrack);

  private:
    QString getFieldString(const QModelIndex& index, const QString& fieldName) const;

    TrackCollection* m_pTrackCollection;
    BansheeDbConnection* m_pConnection;
    int m_playlistId;
};

#endif // BANSHEEPLAYLISTMODEL_H
