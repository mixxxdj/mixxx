#pragma once

#include <QObject>
#include <QString>

#include "library/basesqltablemodel.h"
#include "library/trackmodel.h"

class QModelIndex;

class BaseExternalPlaylistModel : public BaseSqlTableModel {
    Q_OBJECT
  public:
    BaseExternalPlaylistModel(QObject* pParent, TrackCollectionManager* pTrackCollectionManager,
                              const char* settingsNamespace, const QString& playlistsTable,
                              const QString& playlistTracksTable, QSharedPointer<BaseTrackCache> trackSource);

    ~BaseExternalPlaylistModel() override;

    void setPlaylist(const QString& path_name);
    void setPlaylistById(int playlistId);

    TrackPointer getTrack(const QModelIndex& index) const override;
    TrackId getTrackId(const QModelIndex& index) const override;
    bool isColumnInternal(int column) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    Capabilities getCapabilities() const override;
    QString modelKey(bool noSearch) const override;

  private:
    TrackId doGetTrackId(const TrackPointer& pTrack) const override;

    QString m_playlistsTable;
    QString m_playlistTracksTable;
    QSharedPointer<BaseTrackCache> m_trackSource;
    int m_currentPlaylistId;
    QHash<int, QString> m_searchTexts;
};
