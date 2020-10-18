#pragma once

#include <QHash>
#include <QtSql>

#include "library/basesqltablemodel.h"
#include "library/clementine/clementinedbconnection.h"

class ClementinePlaylistModel : public BaseSqlTableModel {
    Q_OBJECT
  public:
    ClementinePlaylistModel(QObject* pParent,
            TrackCollectionManager* pTrackCollectionManager,
            ClementineDbConnection* pConnection);
    ~ClementinePlaylistModel() = default;

    void setTableModel(int playlistId);

    TrackPointer getTrack(const QModelIndex& index) const final;
    TrackId getTrackId(const QModelIndex& index) const final;

    QString getTrackLocation(const QModelIndex& index) const final;
    bool isColumnInternal(int column) final;

    Qt::ItemFlags flags(const QModelIndex& index) const final;
    Capabilities getCapabilities() const final;

  private:
    TrackId doGetTrackId(const TrackPointer& pTrack) const final;

    QString getFieldString(const QModelIndex& index, const QString& fieldName) const;
    QVariant getFieldVariant(const QModelIndex& index, const QString& fieldName) const;

    ClementineDbConnection* m_pConnection;
    int m_playlistId;
};
