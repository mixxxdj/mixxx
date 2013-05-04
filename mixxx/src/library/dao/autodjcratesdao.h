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

class AutoDJCratesDAO : public QObject, public virtual DAO {
    Q_OBJECT
  public:

    AutoDJCratesDAO(QSqlDatabase& a_rDatabase, TrackDAO& a_rTrackDAO,
        CrateDAO& a_rCrateDAO, PlaylistDAO &a_rPlaylistDAO,
        ConfigObject<ConfigValue>* a_pConfig);
    virtual ~AutoDJCratesDAO();

    virtual void initialize();
        // A pure virtual method from the subclass.

    int getRandomTrackId (void);
        // Get the ID of a random track.

  private:

    DISALLOW_COPY_AND_ASSIGN(AutoDJCratesDAO);
        // Disallow copy and assign.
        // (Isn't that normal for QObject subclasses?)
    QSqlDatabase& m_rDatabase;
        // The SQL database we interact with.
    TrackDAO& m_rTrackDAO;
        // The track database-access-object.
        // Used to detect changes to the number of times a track has played.
    CrateDAO& m_rCrateDAO;
        // The crate database-access-object.
        // Used to detect changes to the auto-DJ status of crates, and to get
        // the list of tracks in such crates.
    PlaylistDAO& m_rPlaylistDAO;
        // The playlist database-access-object.
        // Used to detect changes to the auto-DJ playlist.
    ConfigObject<ConfigValue>* m_pConfig;
        // The source of our configuration.
    int *m_pDeckTracks;
        // The track loaded into each deck.
        // This is an array with as many entries as decks.
    int m_iAutoDjPlaylistID;
        // The auto-DJ playlist's ID.
    bool m_bAutoDjCratesDbCreated;
        // True if the auto-DJ-crates database has been created.

    void createAutoDjCratesDatabase();
        // Create the temporary auto-DJ-crates database.
        // Done the first time it's used, since the user might not even make
        // use of this feature.
    bool updateAutoDjPlaylistReferences();
        // Update the number of auto-DJ-playlist references to each track in the
        // auto-DJ-crates database.  Returns true if successful.

  private slots:
    void slotTrackDirty(int a_iTrackId);
        // Signaled by the track DAO when a track's information is updated.
    void slotCrateAdded(int a_iCrateId);
        // Signaled by the crate DAO when a crate is added.
    void slotCrateDeleted(int a_iCrateId);
        // Signaled by the crate DAO when a crate is deleted.
    void slotCrateAutoDjChanged(int a_iCrateId, bool a_bIn);
        // Signaled by the crate DAO when a crate's auto-DJ status changes.
    void slotCrateTrackAdded(int a_iCrateId, int a_iTrackId);
        // Signaled by the crate DAO when a track is added to a crate.
    void slotCrateTrackRemoved(int a_iCrateId, int a_iTrackId);
        // Signaled by the crate DAO when a track is removed from a crate.
    void slotPlaylistTrackAdded(int a_iPlaylistId, int a_iTrackId,
            int a_iPosition);
        // Signaled by the playlist DAO when a track is added to a playlist.
    void slotPlaylistTrackRemoved(int a_iPlaylistId, int a_iTrackId,
            int a_iPosition);
        // Signaled by the playlist DAO when a track is removed from a playlist.
    void slotPlayerInfoTrackLoaded(QString a_strGroup, TrackPointer a_pTrack);
    void slotPlayerInfoTrackUnloaded(QString a_strGroup, TrackPointer a_pTrack);
        // Signaled by the PlayerInfo singleton when a track is loaded to, or
        // unloaded from, a deck.
};

#endif // AUTODJCRATESDAO_H
