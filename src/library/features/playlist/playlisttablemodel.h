#ifndef PLAYLISTTABLEMODEL_H
#define PLAYLISTTABLEMODEL_H
#include <QSet>

#include "library/basesqltablemodel.h"
#include "library/dao/playlistdao.h"

class PlaylistTableModel : public BaseSqlTableModel {
    Q_OBJECT
  public:
    PlaylistTableModel(QObject* parent, TrackCollection* pTrackCollection,
                       const char* settingsNamespace, bool showAll = false);
    ~PlaylistTableModel() final;
    void setTableModel(int playlistId = -1);
    void setTableModel(const QSet<int>& playlistIds);

    int getPlaylist() const {
        return m_iPlaylistId;
    }

    bool appendTrack(TrackId trackId);
    void moveTrack(const QModelIndex& sourceIndex,
                   const QModelIndex& destIndex);
    void removeTrack(const QModelIndex& index);
    void shuffleTracks(const QModelIndexList& shuffle, const QModelIndex& exclude);
    
    void saveSelection(const QModelIndexList& selection);
    QModelIndexList getSavedSelectionIndices();
    
    void select() override;

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
    void playlistChanged(int playlistId);

  private:
    
    int getPosition(const QModelIndex& index);
    
    int m_iPlaylistId;
    QSet<int> m_playlistIds;
    bool m_showAll;
    
    QHash<int, int> m_positionToRow;
    QSet<int> m_savedSelectionIndices;
};

#endif
