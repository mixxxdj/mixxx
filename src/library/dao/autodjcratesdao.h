#ifndef AUTODJCRATESDAO_H
#define AUTODJCRATESDAO_H

#include <QObject>

#include "configobject.h"
#include "trackinfoobject.h"
#include "library/dao/dao.h"
#include "util.h"

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
                    ConfigObject<ConfigValue>* a_pConfig);
    virtual ~AutoDJCratesDAO();

    // A pure virtual method from the subclass.
    virtual void initialize();

    // Get the ID of a random track.
    int getRandomTrackId(void);

  private:

    // Disallow copy and assign.
    // (Isn't that normal for QObject subclasses?)
    DISALLOW_COPY_AND_ASSIGN(AutoDJCratesDAO);

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
    ConfigObject<ConfigValue>* m_pConfig;

    // The track loaded into each deck.
    // This is an array with as many entries as decks.
    int *m_pDeckTracks;

    // The ID of every set-log playlist.
    QList<int> m_lstSetLogPlaylistIds;

    // The auto-DJ playlist's ID.
    int m_iAutoDjPlaylistId;

    // True if active tracks can be tracks that haven't been played in
    // a while.
    bool m_bUseReplayAge;

    // True if the auto-DJ-crates database has been created.
    bool m_bAutoDjCratesDbCreated;

    // Create the temporary auto-DJ-crates database.
    // Done the first time it's used, since the user might not even make
    // use of this feature.
    void createAutoDjCratesDatabase();

    // Create the active-tracks view.
    bool createActiveTracksView(bool a_bUseReplayAge);

    // Update the number of auto-DJ-playlist references to each track in the
    // auto-DJ-crates database.  Returns true if successful.
    bool updateAutoDjPlaylistReferences();

    // Update the number of auto-DJ-playlist references to the given track
    // in the auto-DJ-crates database.  Returns true if successful.
    bool updateAutoDjPlaylistReferencesForTrack(int a_iTrackId);

    // Update the last-played date/time for each track in the
    // auto-DJ-crates database.  Returns true if successful.
    bool updateLastPlayedDateTime();

    // Update the last-played date/time for the given track in the
    // auto-DJ-crates database.  Returns true if successful.
    bool updateLastPlayedDateTimeForTrack(int a_iTrackId);

  private slots:
    // Signaled by the track DAO when a track's information is updated.
    void slotTrackDirty(int a_iTrackId);

    // Signaled by the crate DAO when a crate is added.
    void slotCrateAdded(int a_iCrateId);

    // Signaled by the crate DAO when a crate is deleted.
    void slotCrateDeleted(int a_iCrateId);

    // Signaled by the crate DAO when a crate's auto-DJ status changes.
    void slotCrateAutoDjChanged(int a_iCrateId, bool a_bIn);

    // Signaled by the crate DAO when a track is added to a crate.
    void slotCrateTrackAdded(int a_iCrateId, int a_iTrackId);

    // Signaled by the crate DAO when a track is removed from a crate.
    void slotCrateTrackRemoved(int a_iCrateId, int a_iTrackId);

    // Signaled by the playlist DAO when a playlist is added.
    void slotPlaylistAdded(int a_iPlaylistId);

    // Signaled by the playlist DAO when a playlist is deleted.
    void slotPlaylistDeleted(int a_iPlaylistId);

    // Signaled by the playlist DAO when a track is added to a playlist.
    void slotPlaylistTrackAdded(int a_iPlaylistId, int a_iTrackId,
                                int a_iPosition);

    // Signaled by the playlist DAO when a track is removed from a playlist.
    void slotPlaylistTrackRemoved(int a_iPlaylistId, int a_iTrackId,
                                  int a_iPosition);

    // Signaled by the PlayerInfo singleton when a track is loaded to, or
    // unloaded from, a deck.
    void slotPlayerInfoTrackLoaded(QString a_strGroup, TrackPointer a_pTrack);
    void slotPlayerInfoTrackUnloaded(QString a_strGroup, TrackPointer a_pTrack);
};

#endif // AUTODJCRATESDAO_H
