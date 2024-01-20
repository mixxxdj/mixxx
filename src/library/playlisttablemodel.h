#pragma once

#include "library/trackset/tracksettablemodel.h"
#include "util/duration.h"

class PlaylistTableModel final : public TrackSetTableModel {
    Q_OBJECT

  public:
    PlaylistTableModel(QObject* parent, TrackCollectionManager* pTrackCollectionManager, const char* settingsNamespace, bool keepDeletedTracks = false);
    ~PlaylistTableModel() final = default;

    void selectPlaylist(int playlistId = -1 /* kInvalidPlaylistId */);
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

    /// Get the total duration of all tracks referenced by the given model indices
    mixxx::Duration getTotalDuration(const QModelIndexList& indices);

    Capabilities getCapabilities() const final;

    QString modelKey(bool noSearch) const override;

  private slots:
    void playlistsChanged(const QSet<int>& playlistIds);

  private:
    void initSortColumnMapping() override;

    int m_iPlaylistId;
    bool m_keepDeletedTracks;
    QHash<int, QString> m_searchTexts;
};
