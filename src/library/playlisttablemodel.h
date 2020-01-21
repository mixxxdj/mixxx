#ifndef PLAYLISTTABLEMODEL_H
#define PLAYLISTTABLEMODEL_H

#include "library/basesqltablemodel.h"

class PlaylistTableModel final : public BaseSqlTableModel {
    Q_OBJECT
  public:
    PlaylistTableModel(QObject* parent, TrackCollectionManager* pTrackCollectionManager,
                       const char* settingsNamespace, bool showAll = false);
    ~PlaylistTableModel() final = default;

    void setTableModel(int playlistId = -1);
    int getPlaylist() const {
        return m_iPlaylistId;
    }

    bool appendTrack(TrackId trackId);
    void moveTrack(const QModelIndex& sourceIndex,
                   const QModelIndex& destIndex) override;
    void removeTrack(const QModelIndex& index);
    void shuffleTracks(const QModelIndexList& shuffle, const QModelIndex& exclude);

    bool isColumnInternal(int column) final;
    bool isColumnHiddenByDefault(int column) final;
    // This function should only be used by AUTODJ
    void removeTracks(const QModelIndexList& indices) final;
    // Adding multiple tracks at one to a playlist. Returns the number of
    // successful additions.
    int addTracks(const QModelIndex& index, const QList<QString>& locations) final;
    bool isLocked() final;
    CapabilitiesFlags getCapabilities() const final;

  private slots:
    void playlistsChanged(const QSet<int>& playlistIds);

  private:
    void initSortColumnMapping() override;

    int m_iPlaylistId;
    bool m_showAll;
};

#endif
