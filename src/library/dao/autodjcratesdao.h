#ifndef AUTODJCRATESDAO_H
#define AUTODJCRATESDAO_H

#include <QObject>

#include "preferences/usersettings.h"
#include "library/dao/dao.h"
#include "trackinfoobject.h"
#include "util/class.h"

class QSqlDatabase;
class TrackDAO;
class CrateDAO;
class PlaylistDAO;

#define AUTODJCRATES_TABLE "temp_autodj_crates"
#define AUTODJACTIVETRACKS_TABLE "temp_autodj_activetracks"

#define AUTODJCRATESTABLE_TRACKID "track_id"
#define AUTODJCRATESTABLE_CRATEREFS "craterefs"
#define AUTODJCRATESTABLE_TIMESPLAYED "timesplayed"
#define AUTODJCRATESTABLE_AUTODJREFS "autodjrefs"
#define AUTODJCRATESTABLE_LASTPLAYED "lastplayed"

class AutoDJCratesDAO : public QObject, public virtual DAO {
    Q_OBJECT
  public:

    AutoDJCratesDAO(QSqlDatabase& a_rDatabase, TrackDAO& a_rTrackDAO,
                    CrateDAO& a_rCrateDAO, PlaylistDAO &a_rPlaylistDAO,
                    UserSettingsPointer a_pConfig);
    virtual ~AutoDJCratesDAO();

    // A pure virtual method from the subclass.
    virtual void initialize();

    // Get the ID of a random track.
    int getRandomTrackId(void);

    // Get random track Id from library
    int getRandomTrackIdFromLibrary(const int iPlaylistId);

  private:

    // Disallow copy and assign.
    // (Isn't that normal for QObject subclasses?)
    DISALLOW_COPY_AND_ASSIGN(AutoDJCratesDAO);

    // Create the temporary auto-DJ-crates database.
    // Done the first time it's used, since the user might not even make
    // use of this feature.
    void createAutoDjCratesDatabase();

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

  private slots:
    // Signaled by the track DAO when a track's information is updated.
    void slotTrackDirty(TrackId trackId);

    // Signaled by the crate DAO when a crate is added.
    void slotCrateAdded(int crateId);

    // Signaled by the crate DAO when a crate is deleted.
    void slotCrateDeleted(int crateId);

    // Signaled by the crate DAO when a crate's auto-DJ status changes.
    void slotCrateAutoDjChanged(int crateId, bool added);

    // Signaled by the crate DAO when a track is added to a crate.
    void slotCrateTrackAdded(int crateId, TrackId trackId);

    // Signaled by the crate DAO when a track is removed from a crate.
    void slotCrateTrackRemoved(int crateId, TrackId trackId);

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

    // The SQL database we interact with.
    QSqlDatabase& m_rDatabase;

    // The track database-access-object.
    // Used to detect changes to the number of times a track has played.
    TrackDAO& m_rTrackDAO;

    // The crate database-access-object.
    // Used to detect changes to the auto-DJ status of crates, and to get
    // the list of tracks in such crates.
    CrateDAO& m_rCrateDAO;

    // The playlist database-access-object.
    // Used to detect changes to the auto-DJ playlist.
    PlaylistDAO& m_rPlaylistDAO;

    // The source of our configuration.
    UserSettingsPointer m_pConfig;

    // The ID of every set-log playlist.
    QList<int> m_lstSetLogPlaylistIds;

    // The auto-DJ playlist's ID.
    int m_iAutoDjPlaylistId;

    // True if active tracks can be tracks that haven't been played in
    // a while.
    bool m_bUseIgnoreTime;

    // True if the auto-DJ-crates database has been created.
    bool m_bAutoDjCratesDbCreated;

};

#endif // AUTODJCRATESDAO_H
