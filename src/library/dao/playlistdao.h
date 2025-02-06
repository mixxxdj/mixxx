#pragma once

#include <QHash>
#include <QObject>
#include <QSet>

#include "library/dao/dao.h"
#include "track/trackid.h"
#include "util/class.h"

class AutoDJProcessor;
class QSqlDatabase;

constexpr int kInvalidPlaylistId = -1;

class PlaylistDAO : public QObject, public virtual DAO {
    Q_OBJECT
  public:
    enum HiddenType {
        PLHT_NOT_HIDDEN = 0,
        PLHT_AUTO_DJ = 1,
        PLHT_SET_LOG = 2,
        PLHT_UNKNOWN = -1
    };

    enum class AutoDJSendLoc {
        TOP,
        BOTTOM,
        REPLACE,
    };

    PlaylistDAO();
    ~PlaylistDAO() override = default;

    void initialize(const QSqlDatabase& database) override;

    // Create a playlist, fails with -1 if already exists
    int createPlaylist(const QString& name, const HiddenType type = PLHT_NOT_HIDDEN);
    // Create a playlist, appends "(n)" if already exists, name becomes the new name
    int createUniquePlaylist(QString* pName, const HiddenType type = PLHT_NOT_HIDDEN);
    // Delete a playlist
    void deletePlaylist(const int playlistId);
    // Delete a set of playlists.
    bool deletePlaylists(const QStringList& idStringList);
    bool deleteUnlockedPlaylists(QStringList&& idStringList);
    /// Delete Playlists with fewer entries then "minNumberOfTracks"
    /// Needs to be called inside a transaction.
    /// @return true on success, false on error
    bool deleteAllUnlockedPlaylistsWithFewerTracks(const PlaylistDAO::HiddenType type,
            int minNumberOfTracks);
    // Rename a playlist
    void renamePlaylist(const int playlistId, const QString& newName);
    // Lock or unlock a playlist
    bool setPlaylistLocked(const int playlistId, const bool locked);
    int setPlaylistsLocked(const QSet<int>& playlistIds, const bool lock);
    // Find out the state of a playlist lock
    bool isPlaylistLocked(const int playlistId) const;
    // Append a list of tracks to a playlist
    bool appendTracksToPlaylist(const QList<TrackId>& trackIds, const int playlistId);
    // Append a track to a playlist
    bool appendTrackToPlaylist(TrackId trackId, const int playlistId);
    // Find out how many playlists exist.
    unsigned int playlistCount() const;
    // Get all playlist ids and names of a specific type
    QList<QPair<int, QString>> getPlaylists(const HiddenType hidden) const;
    // Find out the name of the playlist at the given Id
    QString getPlaylistName(const int playlistId) const;
    // Get the playlist id by its name
    int getPlaylistIdFromName(const QString& name) const;
    // Get the id of the playlist at index. Note that the index is the natural
    // position in the database table, not the display order position column
    // stored in the database.
    int getPlaylistId(const int index) const;
    QList<TrackId> getTrackIds(const int playlistId) const;
    // Returns true if the playlist with playlistId is hidden
    bool isHidden(const int playlistId) const;
    // Returns the HiddenType of playlistId
    HiddenType getHiddenType(const int playlistId) const;
    // Returns the maximum position of the given playlist
    int getMaxPosition(const int playlistId) const;
    // Remove a track from all playlists
    void removeTracksFromPlaylists(const QList<TrackId>& trackIds, bool purged = false);
    // removes all hidden and purged Tracks from the playlist
    void removeHiddenTracks(const int playlistId);
    // Remove a track from a playlist
    void removeTrackFromPlaylist(int playlistId, int position);
    void removeTracksFromPlaylist(int playlistId, const QList<int>& positions);
    void removeTracksFromPlaylistById(int playlistId, TrackId trackId);
    // Insert a track into a specific position in a playlist
    bool insertTrackIntoPlaylist(TrackId trackId, int playlistId, int position);
    // Inserts a list of tracks into playlist
    int insertTracksIntoPlaylist(const QList<TrackId>& trackIds, const int playlistId, int position);
    // Remove all tracks from the Auto-DJ Queue
    void clearAutoDJQueue();
    // Add a playlist to the Auto-DJ Queue
    void addPlaylistToAutoDJQueue(const int playlistId, AutoDJSendLoc loc);
    // Add a list of tracks to the Auto-DJ Queue
    void addTracksToAutoDJQueue(const QList<TrackId>& trackIds, AutoDJSendLoc loc);
    // Get the preceding playlist of currentPlaylistId with the HiddenType
    // hidden. Returns -1 if no such playlist exists.
    int getPreviousPlaylist(const int currentPlaylistId, HiddenType hidden) const;
    // Get the following playlist of currentPlaylistId with the HiddenType
    // hidden. Returns -1 if no such playlist exists.
    int getNextPlaylist(const int currentPlaylistId, HiddenType hidden) const;
    // Append all the tracks in the source playlist to the target playlist.
    bool copyPlaylistTracks(const int sourcePlaylistID, const int targetPlaylistID);
    // Returns the number of tracks in the given playlist.
    int tracksInPlaylist(const int playlistId) const;
    // moved Track to a new position
    void moveTrack(const int playlistId,
            const int oldPosition, const int newPosition);
    // shuffles all tracks in the position List
    void shuffleTracks(const int playlistId, const QList<int>& positions, const QHash<int,TrackId>& allIds);
    bool isTrackInPlaylist(TrackId trackId, const int playlistId) const;

    void getPlaylistsTrackIsIn(TrackId trackId, QSet<int>* playlistSet) const;

    void setAutoDJProcessor(AutoDJProcessor* pAutoDJProcessor);

  signals:
    void added(int playlistId);
    void deleted(int playlistId);
    void renamed(int playlistId, const QString& newName);
    void lockChanged(const QSet<int>& playlistIds);
    void trackAdded(int playlistId, TrackId trackId, int position);
    void trackRemoved(int playlistId, TrackId trackId, int position);
    // added / removed / un/locked. Triggers playlist features to update the sidebar
    void playlistContentChanged(const QSet<int>& playlistIds);
    // Separate signals for PlaylistTableModel
    void tracksAdded(const QSet<int>& playlistIds);
    void tracksMoved(const QSet<int>& playlistIds);
    void tracksRemoved(const QSet<int>& playlistIds);
    void tracksRemovedFromPlayedHistory(const QSet<TrackId>& playedTrackIds);

  private:
    bool removeTracksFromPlaylist(int playlistId, int startIndex);
    void removeTracksFromPlaylistInner(int playlistId, int position);
    void removeTracksFromPlaylistByIdInner(int playlistId, TrackId trackId);
    void searchForDuplicateTrack(const int fromPosition,
                                 const int toPosition,
                                 TrackId trackID,
                                 const int excludePosition,
                                 const int otherTrackPosition,
                                 const QHash<int,TrackId>* pTrackPositionIds,
                                 int* pTrackDistance);
    void populatePlaylistMembershipCache();

    QMultiHash<TrackId, int> m_playlistsTrackIsIn;
    AutoDJProcessor* m_pAutoDJProcessor;
    DISALLOW_COPY_AND_ASSIGN(PlaylistDAO);
};
