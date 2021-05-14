#pragma once

#include "library/basesqltablemodel.h"
#include "library/trackset/tracksettablemodel.h"

class PlaylistTableModel final : public TrackSetTableModel {
    Q_OBJECT

  public:
    PlaylistTableModel(QObject* parent, TrackCollectionManager* pTrackCollectionManager, const char* settingsNamespace, bool keepDeletedTracks = false);
    ~PlaylistTableModel() final = default;

    void setTableModel(int playlistId = -1);
    int getPlaylist() const {
        return m_iPlaylistId;
    }

    bool appendTrack(TrackId trackId);
    void moveTrack(const QModelIndex& sourceIndex, const QModelIndex& destIndex) override;
    void removeTrack(const QModelIndex& index);
    void shuffleTracks(const QModelIndexList& shuffle, const QModelIndex& exclude);

    bool isColumnInternal(int column) final;
    bool isColumnHiddenByDefault(int column) final;
    /// This function should only be used by AUTODJ
    void removeTracks(const QModelIndexList& indices) final;
    /// Returns the number of successful additions.
    int addTracks(const QModelIndex& index, const QList<QString>& locations) final;
    bool isLocked() final;

    Capabilities getCapabilities() const final;

  private slots:
    void playlistsChanged(const QSet<int>& playlistIds);

  private:
    void initSortColumnMapping() override;

    int m_iPlaylistId;
    bool m_keepDeletedTracks;
};
