#pragma once

#include <QObject>
#include <QSqlDatabase>

#include "library/crate/crateid.h"
#include "preferences/usersettings.h"
#include "track/track_decl.h"
#include "track/trackid.h"
#include "util/class.h"

class TrackCollection;

class AutoDJCratesDAO : public QObject {
    Q_OBJECT
  public:
    AutoDJCratesDAO(
            int iAutoDjPlaylistId,
            TrackCollection* pTrackCollection,
            UserSettingsPointer a_pConfig);
    ~AutoDJCratesDAO() override;

    // Get the ID of a random track.
    TrackId getRandomTrackId();

    // Get random track Id from library
    TrackId getRandomTrackIdFromLibrary(int iPlaylistId);

  private:

    // Disallow copy and assign.
    // (Isn't that normal for QObject subclasses?)
    DISALLOW_COPY_AND_ASSIGN(AutoDJCratesDAO);

    // Create the temporary auto-DJ-crates database.
    // Done the first time it's used, since the user might not even make
    // use of this feature.
    void createAndConnectAutoDjCratesDatabase();

    // Create the active-tracks view.
    bool createActiveTracksView(bool a_bUseIgnoreTime);

    // Update the number of auto-DJ-playlist references to each track in the
    // auto-DJ-crates database.  Returns true if successful.
    bool updateAutoDjPlaylistReferences();

    // Update the number of auto-DJ-playlist references to the given track
    // in the auto-DJ-crates database.  Returns true if successful.
    bool updateAutoDjPlaylistReferencesForTrack(TrackId trackId);

    // Update the last-played date/time for each track in the
    // auto-DJ-crates database.  Returns true if successful.
    bool updateLastPlayedDateTime();

    // Update the last-played date/time for the given track in the
    // auto-DJ-crates database.  Returns true if successful.
    bool updateLastPlayedDateTimeForTrack(TrackId trackId);

    // Calculates a random Track from AutoDJ,
    // This is used when all active tracks are already queued up.
    TrackId getRandomTrackIdFromAutoDj(int percentActive);

  private slots:
    // Signaled by the track DAO when a track's information is updated.
    void slotTrackDirty(TrackId trackId);

    // Signaled by the crate DAO when a crate is added.
    void slotCrateInserted(CrateId crateId);

    // Signaled by the crate DAO when a crate is updated.
    void slotCrateUpdated(CrateId crateId);

    // Signaled by the crate DAO when a crate is deleted.
    void slotCrateDeleted(CrateId crateId);

    // Signaled by the crate DAO when crate tracks are added/removed.
    void slotCrateTracksChanged(CrateId crateId, const QList<TrackId>& addedTrackIds, const QList<TrackId>& removedTrackIds);

    // Signaled by the playlist DAO when a playlist is added.
    void slotPlaylistAdded(int playlistId);

    // Signaled by the playlist DAO when a playlist is deleted.
    void slotPlaylistDeleted(int playlistId);

    // Signaled by the playlist DAO when a track is added to a playlist.
    void slotPlaylistTrackAdded(int playlistId, TrackId trackId,
                                int position);

    // Signaled by the playlist DAO when a track is removed from a playlist.
    void slotPlaylistTrackRemoved(int playlistId, TrackId trackId,
                                  int position);

    // Signaled by the PlayerInfo singleton when a track is loaded to, or
    // unloaded from, a deck.
    void slotPlayerInfoTrackLoaded(QString group, TrackPointer pTrack);
    void slotPlayerInfoTrackUnloaded(QString group, TrackPointer pTrack);

  private:
    void updateAutoDjCrate(CrateId crateId);
    void deleteAutoDjCrate(CrateId crateId);

    // The auto-DJ playlist's ID.
    const int m_iAutoDjPlaylistId;

    TrackCollection* m_pTrackCollection;

    // The SQL database we interact with.
    QSqlDatabase m_database;

    // The source of our configuration.
    UserSettingsPointer m_pConfig;

    // True if the auto-DJ-crates database has been created.
    bool m_bAutoDjCratesDbCreated;

    // True if active tracks can be tracks that haven't been played in
    // a while.
    bool m_bUseIgnoreTime;

    // The ID of every set-log playlist.
    QList<int> m_lstSetLogPlaylistIds;
};
