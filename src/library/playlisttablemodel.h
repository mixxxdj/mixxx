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
    virtual ~PlaylistTableModel();
    void setTableModel(int playlistId = -1);
    void setTableModel(const QSet<int>& playlistIds);

    int getPlaylist() const {
        return m_iPlaylistId;
    }

    bool isColumnInternal(int column);
    bool isColumnHiddenByDefault(int column);
    // This function should only be used by AUTODJ
    void removeTrack(const QModelIndex& index);
    void removeTracks(const QModelIndexList& indices);
    // Adding multiple tracks at one to a playlist. Returns the number of
    // successful additions.
    int addTracks(const QModelIndex& index, const QList<QString>& locations);
    bool appendTrack(TrackId trackId);
    void moveTrack(const QModelIndex& sourceIndex,
                   const QModelIndex& destIndex);
    bool isLocked();
    void shuffleTracks(const QModelIndexList& shuffle, const QModelIndex& exclude);
    TrackModel::CapabilitiesFlags getCapabilities() const;
    
    void saveSelection(const QModelIndexList& selection);

  private slots:
    void playlistChanged(int playlistId);

  private:
    
    int getPosition(const QModelIndex& index);
    
    PlaylistDAO& m_playlistDao;
    int m_iPlaylistId;
    QSet<int> m_playlistIds;
    bool m_showAll;
    
    QSet<int> m_savedSelection;
};

#endif
