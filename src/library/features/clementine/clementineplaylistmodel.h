#pragma once

#include <QHash>
#include <QtSql>

#include "library/trackmodel.h"
#include "library/trackcollection.h"
#include "library/dao/trackdao.h"
#include "library/features/clementine/clementinedbconnection.h"
#include "library/stardelegate.h"
#include "library/basesqltablemodel.h"

class ClementinePlaylistModel : public BaseSqlTableModel {
    Q_OBJECT
  public:
    ClementinePlaylistModel(QObject* pParent, TrackCollection* pTrackCollection, ClementineDbConnection* pConnection);
    ~ClementinePlaylistModel() final;

    void setTableModel(int playlistId);

    TrackPointer getTrack(const QModelIndex& index) const final;
    QString getTrackLocation(const QModelIndex& index) const final;
    bool isColumnInternal(int column) final;

    Qt::ItemFlags flags(const QModelIndex &index) const final;
    CapabilitiesFlags getCapabilities() const final;

    bool setData(const QModelIndex& index, const QVariant& value, int role=Qt::EditRole) final;

  protected:
    // Use this if you want a model that is read-only.
    Qt::ItemFlags readOnlyFlags(const QModelIndex &index) const final;
    // Use this if you want a model that can be changed
    Qt::ItemFlags readWriteFlags(const QModelIndex &index) const final;

  private slots:
    void tracksChanged(QSet<TrackId> trackIds);
    void trackLoaded(QString group, TrackPointer pTrack);

  private:
    QString getFieldString(const QModelIndex& index, const QString& fieldName) const;
    QVariant getFieldVariant(const QModelIndex& index, const QString& fieldName) const;

    ClementineDbConnection* m_pConnection;
    int m_playlistId;
};
