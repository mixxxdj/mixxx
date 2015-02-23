#include <QtDebug>
#include <QtSql>

#include "playerinfo.h"
#include "playermanager.h"
#include "library/dao/cratedao.h"
#include "library/dao/settingsdao.h"
#include "library/dao/trackdao.h"
#include "library/queryutil.h"
#include "library/trackcollection.h"
#include "library/dao/autodjcratesdao.h"

// Percentage of most and least played tracks to ignore [0,50)
static const int kLeastPreferredPercent = 15;

AutoDJCratesDAO::AutoDJCratesDAO(QSqlDatabase& a_rDatabase,
                                 TrackDAO& a_rTrackDAO, CrateDAO& a_rCrateDAO,
                                 PlaylistDAO &a_rPlaylistDAO,
                                 ConfigObject<ConfigValue>* a_pConfig)
        : m_rDatabase(a_rDatabase),
          m_rTrackDAO(a_rTrackDAO),
          m_rCrateDAO(a_rCrateDAO),
          m_rPlaylistDAO(a_rPlaylistDAO),
          m_pConfig (a_pConfig),
          // Save the ID of the auto-DJ playlist.
          m_iAutoDjPlaylistId(m_rPlaylistDAO.getPlaylistIdFromName(AUTODJ_TABLE)),
          // By default, active tracks are not tracks that haven't been played in
          // a while.
          m_bUseIgnoreTime(false),
          // The database has been created yet.
          m_bAutoDjCratesDbCreated(false) {
}

AutoDJCratesDAO::~AutoDJCratesDAO() {
}

void AutoDJCratesDAO::initialize() {
}

// Create the temporary auto-DJ-crates table.
// Done the first time it's used, since the user might not even make
// use of this feature.
void AutoDJCratesDAO::createAutoDjCratesDatabase() {
    // If the use of tracks that haven't been played in a while has changed,
    // then the active-tracks view must be recreated.
    bool bUseIgnoreTime = (bool) m_pConfig->getValueString(
            ConfigKey("[Auto DJ]", "UseIgnoreTime"), "0").toInt();
    if (m_bAutoDjCratesDbCreated) {
        if (m_bUseIgnoreTime != bUseIgnoreTime) {
            // Do all this in a single transaction.
            ScopedTransaction oTransaction(m_rDatabase);

            // Get rid of the old active-tracks view.
            QSqlQuery oQuery(m_rDatabase);
            oQuery.exec ("DROP VIEW IF EXISTS " AUTODJACTIVETRACKS_TABLE);
            if (!oQuery.exec()) {
                LOG_FAILED_QUERY(oQuery);
                return;
            }

            // Create the new active-tracks view.
            if (!createActiveTracksView(bUseIgnoreTime)) {
                return;
            }

            // Remember the new setting.
            m_bUseIgnoreTime = bUseIgnoreTime;

            // Commit these changes.
            oTransaction.commit();
        }
    } else {
        m_bUseIgnoreTime = bUseIgnoreTime;
    }

    // If this database has already been created, skip this.
    if (m_bAutoDjCratesDbCreated) {
        return;
    }

    // Do all of this in a single transaction.
    ScopedTransaction oTransaction(m_rDatabase);

    // The auto-DJ-crates table contains the track ID, the number of references
    // to that track ID in all of the auto-DJ crates, the number of times that
    // track has been played, and the number of references to the track in the
    // auto-DJ playlist (or in loaded decks).  It filters out tracks that have
    // been deleted from the database (i.e. "hidden" tracks).

    // Create an empty table.
    QSqlQuery oQuery(m_rDatabase);
    // CREATE TEMP TABLE temp_autodj_crates (track_id INTEGER UNIQUE, craterefs INTEGER, timesplayed INTEGER, autodjrefs INTEGER, lastplayed DATETIME);
    //oQuery.exec ("DROP TABLE IF EXISTS " AUTODJCRATES_TABLE);
    QString strQuery("CREATE TEMP TABLE " AUTODJCRATES_TABLE
        " (" AUTODJCRATESTABLE_TRACKID " INTEGER UNIQUE, "
        AUTODJCRATESTABLE_CRATEREFS " INTEGER, "
        AUTODJCRATESTABLE_TIMESPLAYED " INTEGER, "
        AUTODJCRATESTABLE_AUTODJREFS " INTEGER, "
        AUTODJCRATESTABLE_LASTPLAYED " DATETIME)");
    oQuery.prepare(strQuery);
    if (!oQuery.exec()) {
        LOG_FAILED_QUERY(oQuery);
        return;
    }

    // Fill out the first three columns.
    // Supply default values for the last two.
    // INSERT INTO temp_autodj_crates (track_id, craterefs, timesplayed, autodjrefs, lastplayed) SELECT crate_tracks.track_id, COUNT (*), library.timesplayed, 0, "" FROM crate_tracks, library WHERE crate_tracks.crate_id IN (SELECT id FROM crates WHERE autodj = 1) AND crate_tracks.track_id = library.id AND library.mixxx_deleted = 0 GROUP BY crate_tracks.track_id, library.timesplayed;
    strQuery = QString("INSERT INTO " AUTODJCRATES_TABLE
            " (" AUTODJCRATESTABLE_TRACKID ", " AUTODJCRATESTABLE_CRATEREFS ", "
            AUTODJCRATESTABLE_TIMESPLAYED ", " AUTODJCRATESTABLE_AUTODJREFS ", "
            AUTODJCRATESTABLE_LASTPLAYED ") SELECT " CRATE_TRACKS_TABLE
            ".%1 , COUNT (*), " LIBRARY_TABLE ".%2, 0, \"\" FROM "
            CRATE_TRACKS_TABLE ", " LIBRARY_TABLE " WHERE " CRATE_TRACKS_TABLE
            ".%4 IN (SELECT %5 FROM " CRATE_TABLE " WHERE %6 = 1) AND "
            CRATE_TRACKS_TABLE ".%1 = " LIBRARY_TABLE ".%7 AND " LIBRARY_TABLE
            ".%3 == 0 GROUP BY " CRATE_TRACKS_TABLE ".%1, " LIBRARY_TABLE ".%2")
                .arg(CRATETRACKSTABLE_TRACKID, // %1
                     LIBRARYTABLE_TIMESPLAYED, // %2
                     LIBRARYTABLE_MIXXXDELETED, // %3
                     CRATETRACKSTABLE_CRATEID, // %4
                     CRATETABLE_ID, // %5
                     CRATETABLE_AUTODJ_SOURCE, // %6
                     LIBRARYTABLE_ID); // %7
    oQuery.prepare(strQuery);
    if (!oQuery.exec()) {
        LOG_FAILED_QUERY(oQuery);
        return;
    }

    // Fill out the number of auto-DJ-playlist references.
    if (!updateAutoDjPlaylistReferences()) {
        return;
    }

    // Fill out the last-played date/time.
    if (!updateLastPlayedDateTime()) {
        return;
    }

    // Create the active-tracks view.
    //oQuery.exec ("DROP VIEW IF EXISTS " AUTODJACTIVETRACKS_TABLE);
    if (!createActiveTracksView (m_bUseIgnoreTime)) {
        return;
    }

    // Make a list of the IDs of every set-log playlist.
    // SELECT id FROM Playlists WHERE hidden = 2;
    oQuery.prepare(QString("SELECT %1 FROM " PLAYLIST_TABLE " WHERE %2 = %3")
            .arg(PLAYLISTTABLE_ID, // %1
                 PLAYLISTTABLE_HIDDEN, // %2
                 QString::number(PlaylistDAO::PLHT_SET_LOG))); // %3
    if (oQuery.exec()) {
        while (oQuery.next())
            m_lstSetLogPlaylistIds.append(oQuery.value(0).toInt());
    } else {
        LOG_FAILED_QUERY(oQuery);
        return;
    }

    // Now the auto-DJ crates database is initialized.
    // Externally-driven updates to the database from now on are driven by
    // signals.
    oTransaction.commit();

    // Be notified when a track is modified.
    // We only care when the number of times it's been played changes.
    connect(&m_rTrackDAO, SIGNAL(trackDirty(int)),
            this, SLOT(slotTrackDirty(int)));

    // Be notified when the status of crates changes.
    // We only care about the crates labeled as auto-DJ, and tracks added to,
    // and removed from, such crates.
    connect(&m_rCrateDAO, SIGNAL(added(int)),
            this, SLOT(slotCrateAdded(int)));
    connect(&m_rCrateDAO, SIGNAL(deleted(int)),
            this, SLOT(slotCrateDeleted(int)));
    connect(&m_rCrateDAO, SIGNAL(autoDjChanged(int,bool)),
            this, SLOT(slotCrateAutoDjChanged(int,bool)));
    connect(&m_rCrateDAO, SIGNAL(trackAdded(int,int)),
            this, SLOT(slotCrateTrackAdded(int,int)));
    connect(&m_rCrateDAO, SIGNAL(trackRemoved(int,int)),
            this, SLOT(slotCrateTrackRemoved(int,int)));

    // Be notified when playlists are added/removed.
    // We only care about set-log playlists.
    connect(&m_rPlaylistDAO, SIGNAL(added(int)),
            this, SLOT(slotPlaylistAdded(int)));
    connect(&m_rPlaylistDAO, SIGNAL(deleted(int)),
            this, SLOT(slotPlaylistDeleted(int)));

    // Be notified when tracks are added/removed from playlists.
    // We only care about the auto-DJ playlist and the set-log playlists.
    connect(&m_rPlaylistDAO, SIGNAL(trackAdded(int,int,int)),
            this, SLOT(slotPlaylistTrackAdded(int,int,int)));
    connect(&m_rPlaylistDAO, SIGNAL(trackRemoved(int,int,int)),
            this, SLOT(slotPlaylistTrackRemoved(int,int,int)));

    // Be notified when tracks are loaded to, or unloaded from, a deck.
    // These count as auto-DJ references, i.e. prevent the track from being
    // selected randomly.
    connect(&PlayerInfo::instance(), SIGNAL(trackLoaded(QString,TrackPointer)),
            this, SLOT(slotPlayerInfoTrackLoaded(QString,TrackPointer)));
    connect(&PlayerInfo::instance(),
            SIGNAL(trackUnloaded(QString,TrackPointer)),
            this, SLOT(slotPlayerInfoTrackUnloaded(QString,TrackPointer)));

    // Remember that the auto-DJ-crates database has been created.
    m_bAutoDjCratesDbCreated = true;
}

// Create the active-tracks view.
bool AutoDJCratesDAO::createActiveTracksView (bool a_bUseIgnoreTime) {
    // Create the active-tracks view.  This is a list of all tracks loaded into
    // the auto-DJ-crates database, excluding all tracks already in the auto-DJ
    // playlist, sorted by the number of times the track has been played, and
    // limited by either the active percentage or by the number of tracks that
    // have never been played, whichever is larger.
    //
    // At one point, I hoped that I could create this table in a single SQL
    // statement, with a "limit" clause that dynamically updated the size of the
    // view as the state of the system changed.  Unfortunately, that limit
    // clause is only evaluated once.  For posterity, though, here's the monster
    // SQL query that attempted to create that version:
    //
    // CREATE TEMP VIEW temp_autodj_activetracks
    //  AS SELECT * FROM temp_autodj_crates WHERE autodjrefs = 0
    //  ORDER BY timesplayed, lastplayed LIMIT (SELECT MAX(count) FROM
    //  (SELECT COUNT(*) AS count FROM temp_autodj_crates WHERE timesplayed = 0
    //  UNION ALL SELECT (count * (SELECT value FROM settings WHERE
    //  name="mixxx.db.model.autodjcrates.active_percentage") / 100) AS count
    //  FROM (SELECT COUNT(*) AS count FROM temp_autodj_crates)));

    // CREATE TEMP VIEW temp_autodj_activetracks AS SELECT * FROM temp_autodj_crates WHERE autodjrefs = 0 ORDER BY timesplayed, lastplayed;
    QSqlQuery oQuery(m_rDatabase);
    QString strTimesPlayed;
    if (!a_bUseIgnoreTime) {
        strTimesPlayed = AUTODJCRATESTABLE_TIMESPLAYED ", ";
    }
    oQuery.prepare(QString("CREATE TEMP VIEW " AUTODJACTIVETRACKS_TABLE
            " AS SELECT * FROM " AUTODJCRATES_TABLE " WHERE "
            AUTODJCRATESTABLE_AUTODJREFS " = 0 ORDER BY %1"
            AUTODJCRATESTABLE_LASTPLAYED)
            .arg(strTimesPlayed)); // %1
    if (!oQuery.exec()) {
        LOG_FAILED_QUERY(oQuery);
        return false;
    }
    return true;
}

// Update the number of auto-DJ-playlist references to each track in the
// auto-DJ-crates database.
bool AutoDJCratesDAO::updateAutoDjPlaylistReferences() {
    QSqlQuery oQuery(m_rDatabase);

    // Rebuild the auto-DJ-playlist reference count.
    // INSERT OR REPLACE INTO temp_autodj_crates (track_id, craterefs, timesplayed, autodjrefs) SELECT * FROM (SELECT PlaylistTracks.track_id, craterefs, timesplayed, COUNT (*) AS newautodjrefs FROM PlaylistTracks, temp_autodj_crates WHERE PlaylistTracks.playlist_id IN (SELECT id FROM Playlists WHERE hidden = 1) AND PlaylistTracks.track_id = temp_autodj_crates.track_id GROUP BY PlaylistTracks.track_id) WHERE newautodjrefs > 0;
    QString strHidden;
    strHidden.setNum(PlaylistDAO::PLHT_AUTO_DJ);
    QString strQuery(QString ("INSERT OR REPLACE INTO " AUTODJCRATES_TABLE
            " (" AUTODJCRATESTABLE_TRACKID ", " AUTODJCRATESTABLE_CRATEREFS ", "
            AUTODJCRATESTABLE_TIMESPLAYED ", " AUTODJCRATESTABLE_AUTODJREFS ")"
            " SELECT * FROM (SELECT " PLAYLIST_TRACKS_TABLE ".%1, "
            AUTODJCRATESTABLE_CRATEREFS ", " AUTODJCRATESTABLE_TIMESPLAYED
            ", COUNT (*) AS new" AUTODJCRATESTABLE_AUTODJREFS " FROM "
            PLAYLIST_TRACKS_TABLE ", " AUTODJCRATES_TABLE " WHERE "
            PLAYLIST_TRACKS_TABLE ".%2 IN (SELECT %3 FROM " PLAYLIST_TABLE
            " WHERE %4 = %5) AND " PLAYLIST_TRACKS_TABLE ".%1 = "
            AUTODJCRATES_TABLE "." AUTODJCRATESTABLE_TRACKID " GROUP BY "
            PLAYLIST_TRACKS_TABLE ".%1) WHERE new" AUTODJCRATESTABLE_AUTODJREFS
            " > 0")
            .arg(PLAYLISTTRACKSTABLE_TRACKID, // %1
                 PLAYLISTTRACKSTABLE_PLAYLISTID, // %2
                 PLAYLISTTABLE_ID, // %3
                 PLAYLISTTABLE_HIDDEN, // %4
                 strHidden)); // %5
    oQuery.prepare(strQuery);
    if (!oQuery.exec()) {
        LOG_FAILED_QUERY(oQuery);
        return false;
    }

    // Incorporate all tracks loaded into decks.
    // Each track has to be done as a separate database query, in case the same
    // track is loaded into multiple decks.
    int iDecks = (int) PlayerManager::numDecks();
    for (int i = 0; i < iDecks; ++i) {
        QString group = PlayerManager::groupForDeck(i);
        TrackPointer pTrack = PlayerInfo::instance().getTrackInfo(group);
        if (pTrack) {
            int iTrackId = pTrack->getId();
            // UPDATE temp_autodj_crates SET autodjrefs = autodjrefs + 1 WHERE track_id IN (:track_id);
            oQuery.prepare("UPDATE " AUTODJCRATES_TABLE " SET "
                    AUTODJCRATESTABLE_AUTODJREFS " = " AUTODJCRATESTABLE_AUTODJREFS
                    " + 1 WHERE " AUTODJCRATESTABLE_TRACKID " IN (:track_id)");
            oQuery.bindValue(":track_id", iTrackId);
            if (!oQuery.exec()) {
                LOG_FAILED_QUERY(oQuery);
                return false;
            }
        }
    }
    return true;
}

// Update the number of auto-DJ-playlist references to the given track in the
// auto-DJ-crates database.
bool AutoDJCratesDAO::updateAutoDjPlaylistReferencesForTrack(int trackId) {
    QSqlQuery oQuery(m_rDatabase);

    // INSERT OR REPLACE INTO temp_autodj_crates (track_id, craterefs, timesplayed, autodjrefs) SELECT * FROM (SELECT :track_id AS new_track_id, craterefs, timesplayed, COUNT (*) AS newautodjrefs FROM PlaylistTracks, temp_autodj_crates WHERE PlaylistTracks.playlist_id IN (SELECT id FROM Playlists WHERE hidden = 1) AND PlaylistTracks.track_id = :track_id AND temp_autodj_crates.track_id = :track_id GROUP BY new_track_id) WHERE newautodjrefs > 0;
    QString strHidden;
    strHidden.setNum(PlaylistDAO::PLHT_AUTO_DJ);
    oQuery.prepare(QString("INSERT OR REPLACE INTO " AUTODJCRATES_TABLE " ("
            AUTODJCRATESTABLE_TRACKID ", " AUTODJCRATESTABLE_CRATEREFS ", "
            AUTODJCRATESTABLE_TIMESPLAYED ", " AUTODJCRATESTABLE_AUTODJREFS
            ") SELECT * FROM (SELECT :track_id_1 AS new_track_id, "
            AUTODJCRATESTABLE_CRATEREFS ", " AUTODJCRATESTABLE_TIMESPLAYED
            ", COUNT (*) AS new" AUTODJCRATESTABLE_AUTODJREFS " FROM "
            PLAYLIST_TRACKS_TABLE ", " AUTODJCRATES_TABLE " WHERE "
            PLAYLIST_TRACKS_TABLE ".%1 IN (SELECT %4 FROM " PLAYLIST_TABLE
            " WHERE %2 = %5) AND " PLAYLIST_TRACKS_TABLE ".%3 = :track_id_2 AND "
            AUTODJCRATES_TABLE "." AUTODJCRATESTABLE_TRACKID
            " = :track_id_3 GROUP BY new_track_id) WHERE new"
            AUTODJCRATESTABLE_AUTODJREFS " > 0")
            .arg(PLAYLISTTRACKSTABLE_PLAYLISTID, // %1
                 PLAYLISTTABLE_HIDDEN, // %2
                 PLAYLISTTRACKSTABLE_TRACKID, // %3
                 PLAYLISTTABLE_ID, // %4
                 strHidden)); // %5
    oQuery.bindValue(":track_id_1", trackId);
    oQuery.bindValue(":track_id_2", trackId);
    oQuery.bindValue(":track_id_3", trackId);
    if (!oQuery.exec()) {
        LOG_FAILED_QUERY(oQuery);
        return false;
    }

    // The update was successful.
    return true;
}

// Update the last-played date/time for each track in the auto-DJ-crates
// database.
bool AutoDJCratesDAO::updateLastPlayedDateTime() {
    QSqlQuery oQuery(m_rDatabase);

    // Rebuild the auto-DJ-playlist last-played date/time.
    // INSERT OR REPLACE INTO temp_autodj_crates (track_id, craterefs, timesplayed, autodjrefs, lastplayed) SELECT * FROM (SELECT PlaylistTracks.track_id, craterefs, timesplayed, autodjrefs, MAX(pl_datetime_added) AS newlastplayed FROM PlaylistTracks, temp_autodj_crates WHERE PlaylistTracks.playlist_id IN (SELECT id FROM Playlists WHERE hidden = 2) AND PlaylistTracks.track_id = temp_autodj_crates.track_id GROUP BY PlaylistTracks.track_id) WHERE newlastplayed != "";
    QString strSetLog;
    strSetLog.setNum(PlaylistDAO::PLHT_SET_LOG);
    QString strQuery(QString ("INSERT OR REPLACE INTO " AUTODJCRATES_TABLE
            " (" AUTODJCRATESTABLE_TRACKID ", " AUTODJCRATESTABLE_CRATEREFS ", "
            AUTODJCRATESTABLE_TIMESPLAYED ", " AUTODJCRATESTABLE_AUTODJREFS ", "
            AUTODJCRATESTABLE_LASTPLAYED ")"
            " SELECT * FROM (SELECT " PLAYLIST_TRACKS_TABLE ".%1, "
            AUTODJCRATESTABLE_CRATEREFS ", " AUTODJCRATESTABLE_TIMESPLAYED ", "
            AUTODJCRATESTABLE_AUTODJREFS ", MAX(%3) AS new"
            AUTODJCRATESTABLE_LASTPLAYED " FROM " PLAYLIST_TRACKS_TABLE ", "
            AUTODJCRATES_TABLE " WHERE " PLAYLIST_TRACKS_TABLE
            ".%2 IN (SELECT %4 FROM " PLAYLIST_TABLE " WHERE %5 = %6) AND "
            PLAYLIST_TRACKS_TABLE ".%1 = " AUTODJCRATES_TABLE "."
            AUTODJCRATESTABLE_TRACKID " GROUP BY " PLAYLIST_TRACKS_TABLE
            ".%1) WHERE new" AUTODJCRATESTABLE_LASTPLAYED " != \"\"")
            .arg(PLAYLISTTRACKSTABLE_TRACKID, // %1
                 PLAYLISTTRACKSTABLE_PLAYLISTID, // %2
                 PLAYLISTTRACKSTABLE_DATETIMEADDED, // %3
                 PLAYLISTTABLE_ID, // %4
                 PLAYLISTTABLE_HIDDEN, // %5
                 strSetLog)); // %6
    oQuery.prepare(strQuery);
    if (!oQuery.exec()) {
        LOG_FAILED_QUERY(oQuery);
        return false;
    }

    return true;
}

// Update the last-played date/time for the given track in the auto-DJ-crates
// database.
bool AutoDJCratesDAO::updateLastPlayedDateTimeForTrack(int trackId) {
    QSqlQuery oQuery(m_rDatabase);

    // Update the last-played date/time for this track.
    // INSERT OR REPLACE INTO temp_autodj_crates (track_id, craterefs, timesplayed, autodjrefs, lastplayed) SELECT * FROM (SELECT PlaylistTracks.track_id, craterefs, timesplayed, autodjrefs, MAX(pl_datetime_added) AS newlastplayed FROM PlaylistTracks, temp_autodj_crates WHERE PlaylistTracks.playlist_id IN (SELECT id FROM Playlists WHERE hidden = 2) AND PlaylistTracks.track_id = :track_id AND PlaylistTracks.track_id = temp_autodj_crates.track_id GROUP BY PlaylistTracks.track_id) WHERE newlastplayed != "";
    QString strSetLog;
    strSetLog.setNum(PlaylistDAO::PLHT_SET_LOG);
    oQuery.prepare(QString ("INSERT OR REPLACE INTO " AUTODJCRATES_TABLE
            " (" AUTODJCRATESTABLE_TRACKID ", " AUTODJCRATESTABLE_CRATEREFS ", "
            AUTODJCRATESTABLE_TIMESPLAYED ", " AUTODJCRATESTABLE_AUTODJREFS ", "
            AUTODJCRATESTABLE_LASTPLAYED ")"
            " SELECT * FROM (SELECT " PLAYLIST_TRACKS_TABLE ".%1, "
            AUTODJCRATESTABLE_CRATEREFS ", " AUTODJCRATESTABLE_TIMESPLAYED ", "
            AUTODJCRATESTABLE_AUTODJREFS ", MAX(%3) AS new"
            AUTODJCRATESTABLE_LASTPLAYED " FROM " PLAYLIST_TRACKS_TABLE ", "
            AUTODJCRATES_TABLE " WHERE " PLAYLIST_TRACKS_TABLE
            ".%2 IN (SELECT %4 FROM " PLAYLIST_TABLE " WHERE %5 = %6) AND "
            PLAYLIST_TRACKS_TABLE ".%1 = :track_id AND " PLAYLIST_TRACKS_TABLE
            ".%1 = " AUTODJCRATES_TABLE "." AUTODJCRATESTABLE_TRACKID
            " GROUP BY " PLAYLIST_TRACKS_TABLE ".%1) WHERE new"
            AUTODJCRATESTABLE_LASTPLAYED " != \"\"")
            .arg(PLAYLISTTRACKSTABLE_TRACKID, // %1
                 PLAYLISTTRACKSTABLE_PLAYLISTID, // %2
                 PLAYLISTTRACKSTABLE_DATETIMEADDED, // %3
                 PLAYLISTTABLE_ID, // %4
                 PLAYLISTTABLE_HIDDEN, // %5
                 strSetLog)); // %6
    oQuery.bindValue(":track_id", trackId);
    if (!oQuery.exec()) {
        LOG_FAILED_QUERY(oQuery);
        return false;
    }

    // The update was successful.
    return true;
}

// Get the ID, i.e. one that references library.id, of a random track.
// Returns -1 if there was an error.
int AutoDJCratesDAO::getRandomTrackId(void) {
    // If necessary, create the temporary auto-DJ-crates database.
    createAutoDjCratesDatabase();

    // Calculate the number of active-tracks that have never been played, and
    // the total number of active-tracks.
    QSqlQuery oQuery(m_rDatabase);
    // SELECT COUNT(*) AS count FROM temp_autodj_activetracks WHERE timesplayed = 0 UNION ALL SELECT COUNT(*) AS count FROM temp_autodj_activetracks;
    int iUnplayedTracks = 0, iTotalTracks = 0;
    oQuery.prepare("SELECT COUNT(*) AS count FROM " AUTODJACTIVETRACKS_TABLE
        " WHERE " AUTODJCRATESTABLE_TIMESPLAYED
        " = 0 UNION ALL SELECT COUNT(*) AS count FROM "
        AUTODJACTIVETRACKS_TABLE);
    if (oQuery.exec()) {
        if (oQuery.next()) {
            iUnplayedTracks = oQuery.value(0).toInt();
            if (oQuery.next())
                iTotalTracks = oQuery.value(0).toInt();
        }
    } else {
        LOG_FAILED_QUERY(oQuery);
        return -1;
    }

    // Get the active percentage (default 20%).
    int iMinimumAvailable = m_pConfig->getValueString (ConfigKey("[Auto DJ]",
        "MinimumAvailable"), "20").toInt();

    // Calculate the number of active-tracks.  This is either the number of
    // auto-DJ-crate tracks that have never been played, or the active
    // percentage of the total number of tracks, whichever is larger.
    int iMinAvailable = 0;
    if (iMinimumAvailable) {
        // if minimum is not disabled (= 0), have a min of one at least
        iMinAvailable = qMax((iTotalTracks * iMinimumAvailable / 100), 1);
    }
    int iActiveTracks = qMax(iUnplayedTracks, iMinAvailable);

    // The number of active-tracks might also be tracks that haven't been played
    // in a while.
    if (m_bUseIgnoreTime) {
        // Get the current time, in UTC (since that's what sqlite uses).
        QDateTime timCurrent = QDateTime::currentDateTime().toUTC();

        // Subtract the replay age.
        QTime timIgnoreTime = (QTime::fromString(m_pConfig->getValueString
            (ConfigKey("[Auto DJ]", "IgnoreTime"), "23:59"), "hh:mm"));
        timCurrent = timCurrent.addSecs(-(timIgnoreTime.hour() * 3600
            + timIgnoreTime.minute() * 60));

        // Convert the time to sqlite's format, which is similar to ISO date,
        // but not quite.
        QString strDateTime = timCurrent.toString("yyyy-MM-dd hh:mm:ss");

        // Count the number of tracks that haven't been played since this time.
        // SELECT COUNT(*) FROM temp_autodj_activetracks WHERE lastplayed < :lastplayed;
        int iIgnoreTimeTracks = 0;
        oQuery.prepare("SELECT COUNT(*) FROM " AUTODJACTIVETRACKS_TABLE
            " WHERE " AUTODJCRATESTABLE_LASTPLAYED " < :lastplayed");
        oQuery.bindValue (":lastplayed", strDateTime);
        if (oQuery.exec()) {
            if (oQuery.next()) {
                iIgnoreTimeTracks = oQuery.value(0).toInt();
            }
        } else {
            LOG_FAILED_QUERY(oQuery);
            return -1;
        }

        // Allow that to be a new maximum.
        iActiveTracks = qMax(iActiveTracks, iIgnoreTimeTracks);
    }

    // If there are no tracks, let our caller know.
    if (iActiveTracks == 0)
        return -1;

    // Pick a random track.
    // SELECT track_id FROM temp_autodj_activetracks LIMIT 1 OFFSET ABS (RANDOM() % :active);
    oQuery.prepare("SELECT " AUTODJCRATESTABLE_TRACKID " FROM "
        AUTODJACTIVETRACKS_TABLE " LIMIT 1 OFFSET ABS (RANDOM() % :active)");
    oQuery.bindValue (":active", iActiveTracks);
    if (oQuery.exec()) {
        if (oQuery.next()) {
            // Give our caller the randomly-selected track.
            return oQuery.value(0).toInt();
        }
    } else {
        LOG_FAILED_QUERY(oQuery);
    }

    // Let our caller know that a random track couldn't be picked.
    return -1;
}

// Signaled by the track DAO when a track's information is updated.
void AutoDJCratesDAO::slotTrackDirty(int trackId) {
    // Update our record of the number of times played, if that changed.
    TrackPointer pTrack = m_rTrackDAO.getTrack(trackId);
    if (pTrack == NULL) {
        return;
    }
    int iPlayed = pTrack->getTimesPlayed();
    if (iPlayed == 0) {
        return;
    }

    // Update our record of how many times this track has been played.
    // UPDATE temp_autodj_crates SET timesplayed = :newplayed WHERE track_id = :track_id AND timesplayed = :oldplayed;
    QSqlQuery oQuery(m_rDatabase);
    oQuery.prepare("UPDATE " AUTODJCRATES_TABLE " SET "
            AUTODJCRATESTABLE_TIMESPLAYED " = :newplayed WHERE "
            AUTODJCRATESTABLE_TRACKID " = :track_id AND "
            AUTODJCRATESTABLE_TIMESPLAYED " = :oldplayed");
    oQuery.bindValue(":track_id", trackId);
    oQuery.bindValue(":oldplayed", iPlayed - 1);
    oQuery.bindValue(":newplayed", iPlayed);
    if (!oQuery.exec()) {
        LOG_FAILED_QUERY(oQuery);
        return;
    }
}

// Signaled by the crate DAO when a crate is added.
void AutoDJCratesDAO::slotCrateAdded(int crateId) {
    // If this newly-added crate is in the auto-DJ queue, add it to the list.
    if (m_rCrateDAO.isCrateInAutoDj(crateId)) {
        slotCrateAutoDjChanged(crateId, true);
    }
}

void AutoDJCratesDAO::slotCrateDeleted(int crateId) {
    // The crate can't be queried for its auto-DJ status, because it's been
    // deleted by the time this code is reached.  But we can handle that.
    // Another solution would be to add a "crateDeleting" signal to CrateDAO.
    slotCrateAutoDjChanged(crateId, false);
}

void AutoDJCratesDAO::slotCrateAutoDjChanged(int crateId, bool added) {
    // Handle a crate that's entered the auto-DJ queue differently than one that
    // is leaving it.  (Obviously.)
    ScopedTransaction oTransaction(m_rDatabase);
    if (added) {
        // Add a crate-reference to every track in this crate, if that track is
        // already in the auto-DJ-crates table.
        QSqlQuery oQuery(m_rDatabase);
        // UPDATE temp_autodj_crates SET craterefs = craterefs + 1 WHERE track_id IN (SELECT temp_autodj_crates.track_id FROM crate_tracks, temp_autodj_crates WHERE crate_tracks.crate_id = :crate_id AND crate_tracks.track_id = temp_autodj_crates.track_id);
        oQuery.prepare(QString ("UPDATE " AUTODJCRATES_TABLE " SET "
                AUTODJCRATESTABLE_CRATEREFS " = " AUTODJCRATESTABLE_CRATEREFS
                " + 1 WHERE " AUTODJCRATESTABLE_TRACKID " IN (SELECT "
                AUTODJCRATES_TABLE "." AUTODJCRATESTABLE_TRACKID " FROM "
                CRATE_TRACKS_TABLE ", " AUTODJCRATES_TABLE " WHERE "
                CRATE_TRACKS_TABLE ".%2 = :crate_id AND " CRATE_TRACKS_TABLE
                ".%1 = " AUTODJCRATES_TABLE "." AUTODJCRATESTABLE_TRACKID ")")
                .arg(CRATETRACKSTABLE_TRACKID, // %1
                     CRATETRACKSTABLE_CRATEID)); // %2
        oQuery.bindValue(":crate_id", crateId);
        if (!oQuery.exec()) {
            LOG_FAILED_QUERY(oQuery);
            return;
        }

        // Create an entry for all tracks that weren't in the auto-DJ-crates
        // table already.
        // The number of crate references is known to be 1 for such tracks.
        // The number of references to each track in the auto-DJ playlist isn't
        // set yet; it defaults to zero.
        // If no records were modified by this query, then there's no reason to
        // update the number of auto-DJ-playlist references to each track.
        // INSERT INTO temp_autodj_crates (track_id, craterefs, timesplayed, autodjrefs) SELECT crate_tracks.track_id, 1, library.timesplayed, 0 FROM crate_tracks, library WHERE crate_tracks.crate_id = :crate_id AND crate_tracks.track_id NOT IN (SELECT track_id FROM temp_autodj_crates) AND crate_tracks.track_id = library.id AND library.mixxx_deleted = 0;
        oQuery.prepare(QString("INSERT INTO " AUTODJCRATES_TABLE " ("
                AUTODJCRATESTABLE_TRACKID ", " AUTODJCRATESTABLE_CRATEREFS ", "
                AUTODJCRATESTABLE_TIMESPLAYED ", " AUTODJCRATESTABLE_AUTODJREFS
                ") SELECT " CRATE_TRACKS_TABLE ".%1, 1, " LIBRARY_TABLE
                ".timesplayed, 0 FROM " CRATE_TRACKS_TABLE ", " LIBRARY_TABLE
                " WHERE " CRATE_TRACKS_TABLE ".%2 = :crate_id AND "
                CRATE_TRACKS_TABLE ".%1 NOT IN (SELECT " AUTODJCRATESTABLE_TRACKID
                " FROM " AUTODJCRATES_TABLE" ) AND " CRATE_TRACKS_TABLE ".%1 = "
                LIBRARY_TABLE ".%3 AND " LIBRARY_TABLE ".%4 = 0")
                .arg(CRATETRACKSTABLE_TRACKID, // %1
                     CRATETRACKSTABLE_CRATEID, // %2
                     LIBRARYTABLE_ID, // %3
                     LIBRARYTABLE_MIXXXDELETED)); // %4
        oQuery.bindValue(":crate_id", crateId);
        if (!oQuery.exec()) {
            LOG_FAILED_QUERY(oQuery);
            return;
        }
        if (oQuery.numRowsAffected() == 0) {
            oTransaction.commit();
            return;
        }

        // Update the number of auto-DJ-playlist references to each track.
        // There's no way to avoid updating the number of references to tracks
        // that were already in the database, but the auto-DJ-playlist is likely
        // to be small compared to the number of tracks in auto-DJ crates, so
        // this isn't so bad.
        if (!updateAutoDjPlaylistReferences()) {
            return;
        }

        // Update the last-played date/time for each track.
        // Similarly, there's no way to avoid updating the last-played date/time
        // of tracks that were already in the database.
        if (!updateLastPlayedDateTime()) {
            return;
        }

        // The transaction was successful.
        oTransaction.commit();
    } else {
        // Remove a crate-reference from every track in this crate.
        QSqlQuery oQuery(m_rDatabase);
        // UPDATE temp_autodj_crates SET craterefs = craterefs - 1 WHERE track_id IN (SELECT track_id FROM crate_tracks WHERE crate_tracks.crate_id = :crate_id);
        oQuery.prepare(QString("UPDATE " AUTODJCRATES_TABLE " SET "
                AUTODJCRATESTABLE_CRATEREFS " = " AUTODJCRATESTABLE_CRATEREFS
                " - 1 WHERE " AUTODJCRATESTABLE_TRACKID " IN (SELECT %1 FROM "
                CRATE_TRACKS_TABLE " WHERE " CRATE_TRACKS_TABLE ".%2 = :crate_id)")
                .arg(CRATETRACKSTABLE_TRACKID, // %1
                     CRATETRACKSTABLE_CRATEID)); // %2
        oQuery.bindValue(":crate_id", crateId);
        if (!oQuery.exec()) {
            LOG_FAILED_QUERY(oQuery);
            return;
        }

        // Remove all tracks that no longer have crate references.
        //DELETE FROM temp_autodj_crates WHERE craterefs = 0;
        oQuery.prepare("DELETE FROM " AUTODJCRATES_TABLE " WHERE "
            AUTODJCRATESTABLE_CRATEREFS " = 0");
        if (!oQuery.exec()) {
            LOG_FAILED_QUERY(oQuery);
            return;
        }

        // The transaction was successful.
        oTransaction.commit();
    }
}

void AutoDJCratesDAO::slotCrateTrackAdded(int a_iCrateId, int a_iTrackId) {
    // Skip this if it's not an auto-DJ crate.
    if (!m_rCrateDAO.isCrateInAutoDj(a_iCrateId)) {
        return;
    }

    // Add a crate-reference to this track, if it's already in the
    // auto-DJ-crates table (in which case, we're done).
    ScopedTransaction oTransaction(m_rDatabase);
    QSqlQuery oQuery(m_rDatabase);
    // UPDATE temp_autodj_crates SET craterefs = craterefs + 1 WHERE track_id = :track_id;
    QString strHidden;
    strHidden.setNum(PlaylistDAO::PLHT_AUTO_DJ);
    oQuery.prepare("UPDATE " AUTODJCRATES_TABLE " SET "
        AUTODJCRATESTABLE_CRATEREFS " = " AUTODJCRATESTABLE_CRATEREFS
        " + 1 WHERE " AUTODJCRATESTABLE_TRACKID " = :track_id");
    oQuery.bindValue(":track_id", a_iTrackId);
    if (!oQuery.exec()) {
        LOG_FAILED_QUERY(oQuery);
        return;
    }
    if (oQuery.numRowsAffected() == 1) {
        oTransaction.commit();
        return;
    }

    // Create an entry for the track.
    // The number of crate references is known to be 1 for such tracks.
    // The number of references to each track in the auto-DJ playlist isn't
    // set yet; it defaults to zero.
    // If no records were modified by this query (i.e. because the track's
    // mixxx_deleted flag is set), then there's no reason to update the number
    // of auto-DJ-playlist references to each track.
    // INSERT INTO temp_autodj_crates (track_id, craterefs, timesplayed, autodjrefs) SELECT :track_id, 1, library.timesplayed, 0 FROM library WHERE :track_id = library.id AND library.mixxx_deleted = 0;
    oQuery.prepare(QString("INSERT INTO " AUTODJCRATES_TABLE " ("
            AUTODJCRATESTABLE_TRACKID ", " AUTODJCRATESTABLE_CRATEREFS ", "
            AUTODJCRATESTABLE_TIMESPLAYED ", " AUTODJCRATESTABLE_AUTODJREFS
            ") SELECT :track_id_1, 1, " LIBRARY_TABLE ".%1, 0 FROM " LIBRARY_TABLE
            " WHERE :track_id_2 = " LIBRARY_TABLE ".%2 AND " LIBRARY_TABLE
            ".%3 = 0")
            .arg(LIBRARYTABLE_TIMESPLAYED, // %1
                 LIBRARYTABLE_ID, // %2
                 LIBRARYTABLE_MIXXXDELETED)); // %3
    oQuery.bindValue(":track_id_1", a_iTrackId);
    oQuery.bindValue(":track_id_2", a_iTrackId);
    if (!oQuery.exec()) {
        LOG_FAILED_QUERY(oQuery);
        return;
    }
    if (oQuery.numRowsAffected() == 0) {
        oTransaction.commit();
        return;
    }

    // Update the number of auto-DJ-playlist references to this track.
    if (!updateAutoDjPlaylistReferencesForTrack(a_iTrackId)) {
        return;
    }

    // Update the last-played date/time for this track.
    if (!updateLastPlayedDateTimeForTrack(a_iTrackId)) {
        return;
    }

    // The transaction was successful.
    oTransaction.commit();
}

void AutoDJCratesDAO::slotCrateTrackRemoved(int crateId, int trackId) {
    // Skip this if it's not an auto-DJ crate.
    if (!m_rCrateDAO.isCrateInAutoDj(crateId))
        return;

    // Remove a crate-reference from this track.
    ScopedTransaction oTransaction(m_rDatabase);
    QSqlQuery oQuery(m_rDatabase);
    // UPDATE temp_autodj_crates SET craterefs = craterefs - 1 WHERE track_id = :track_id;
    oQuery.prepare("UPDATE " AUTODJCRATES_TABLE " SET "
        AUTODJCRATESTABLE_CRATEREFS " = " AUTODJCRATESTABLE_CRATEREFS
        " - 1 WHERE " AUTODJCRATESTABLE_TRACKID " = :track_id");
    oQuery.bindValue(":track_id", trackId);
    if (!oQuery.exec()) {
        LOG_FAILED_QUERY(oQuery);
        return;
    }

    // Remove the track if it no longer has a crate reference.
    //DELETE FROM temp_autodj_crates WHERE track_id = :track_id AND craterefs = 0;
    oQuery.prepare("DELETE FROM " AUTODJCRATES_TABLE " WHERE "
        AUTODJCRATESTABLE_TRACKID " = :track_id AND "
        AUTODJCRATESTABLE_CRATEREFS " = 0");
    oQuery.bindValue(":track_id", trackId);
    if (!oQuery.exec()) {
        LOG_FAILED_QUERY(oQuery);
        return;
    }

    // The transaction was successful.
    oTransaction.commit();
}

// Signaled by the playlistDAO when a playlist is added.
void AutoDJCratesDAO::slotPlaylistAdded(int playlistId) {
    // We only care about changes to set-log playlists.
    if (m_rPlaylistDAO.getHiddenType(playlistId)
            == PlaylistDAO::PLHT_SET_LOG) {
        m_lstSetLogPlaylistIds.append(playlistId);
        updateLastPlayedDateTime();
    }
}

// Signaled by the playlistDAO when a playlist is deleted.
void AutoDJCratesDAO::slotPlaylistDeleted(int playlistId) {
    // We only care about changes to set-log playlists.
    int iIndex = m_lstSetLogPlaylistIds.indexOf(playlistId);
    if (iIndex >= 0) {
        m_lstSetLogPlaylistIds.removeAt(iIndex);
        updateLastPlayedDateTime();
    }
}

// Signaled by the playlist DAO when a track is added to a playlist.
void AutoDJCratesDAO::slotPlaylistTrackAdded(int playlistId, int trackId,
                                             int /* a_iPosition */) {
    // Deal with changes to the auto-DJ playlist.
    if (playlistId == m_iAutoDjPlaylistId) {
        QSqlQuery oQuery(m_rDatabase);
        // UPDATE temp_autodj_crates SET autodjrefs = autodjrefs + 1 WHERE track_id = :track_id;
        oQuery.prepare("UPDATE " AUTODJCRATES_TABLE " SET "
            AUTODJCRATESTABLE_AUTODJREFS " = " AUTODJCRATESTABLE_AUTODJREFS
            " + 1 WHERE " AUTODJCRATESTABLE_TRACKID " = :track_id");
        oQuery.bindValue(":track_id", trackId);
        if (!oQuery.exec()) {
            LOG_FAILED_QUERY(oQuery);
            return;
        }
    } else if (m_lstSetLogPlaylistIds.contains(playlistId)) {
        // Deal with changes to set-log playlists.
        // If this query doesn't succeed, it'll log a message.
        // Do nothing special otherwise -- any change it makes can be part of
        // any current transaction.
        updateLastPlayedDateTimeForTrack(trackId);
    }
}

// Signaled by the playlist DAO when a track is removed from a playlist.
void AutoDJCratesDAO::slotPlaylistTrackRemoved(int playlistId,
                                               int trackId,
                                               int /* a_iPosition */) {
    // Deal with changes to the auto-DJ playlist.
    if (playlistId == m_iAutoDjPlaylistId) {
        QSqlQuery oQuery(m_rDatabase);
        // UPDATE temp_autodj_crates SET autodjrefs = autodjrefs - 1 WHERE track_id = :track_id;
        oQuery.prepare("UPDATE " AUTODJCRATES_TABLE " SET "
            AUTODJCRATESTABLE_AUTODJREFS " = " AUTODJCRATESTABLE_AUTODJREFS
            " - 1 WHERE " AUTODJCRATESTABLE_TRACKID " = :track_id");
        oQuery.bindValue(":track_id", trackId);
        if (!oQuery.exec()) {
            LOG_FAILED_QUERY(oQuery);
            return;
        }
    } else if (m_lstSetLogPlaylistIds.contains(playlistId)) {
        // Deal with changes to set-log playlists.
        // If this query doesn't succeed, it'll log a message.
        // Do nothing special otherwise -- any change it makes can be part of
        // any current transaction.
        updateLastPlayedDateTimeForTrack(trackId);
    }
}

// Signaled by the PlayerInfo singleton when a track is loaded to a deck.
void AutoDJCratesDAO::slotPlayerInfoTrackLoaded(QString a_strGroup,
                                                TrackPointer a_pTrack) {
    // This gets called with a null track during an unload.  Filter that out.
    if (a_pTrack == NULL) {
        return;
    }

    // This counts as an auto-DJ reference.  The idea is to prevent tracks that
    // are loaded into a deck from being randomly chosen.
    int iTrackId = a_pTrack->getId();
    unsigned int numDecks = PlayerManager::numDecks();
    for (unsigned int i = 0; i < numDecks; ++i) {
        if (a_strGroup == PlayerManager::groupForDeck(i)) {
            // Update the number of auto-DJ-playlist references to this track.
            QSqlQuery oQuery(m_rDatabase);
            // UPDATE temp_autodj_crates SET autodjrefs = autodjrefs + 1 WHERE track_id = :track_id;
            oQuery.prepare("UPDATE " AUTODJCRATES_TABLE " SET "
                AUTODJCRATESTABLE_AUTODJREFS " = " AUTODJCRATESTABLE_AUTODJREFS
                " + 1 WHERE " AUTODJCRATESTABLE_TRACKID " = :track_id");
            oQuery.bindValue(":track_id", iTrackId);
            if (!oQuery.exec()) {
                LOG_FAILED_QUERY(oQuery);
                return;
            }
            return;
        }
    }
}

// Signaled by the PlayerInfo singleton when a track is unloaded from a deck.
void AutoDJCratesDAO::slotPlayerInfoTrackUnloaded(QString group,
                                                  TrackPointer pTrack) {
    // This counts as an auto-DJ reference.  The idea is to prevent tracks that
    // are loaded into a deck from being randomly chosen.
    int iTrackId = pTrack->getId();
    unsigned int numDecks = PlayerManager::numDecks();
    for (unsigned int i = 0; i < numDecks; ++i) {
        if (group == PlayerManager::groupForDeck(i)) {
            // Get rid of the ID of the track in this deck.
            QSqlQuery oQuery(m_rDatabase);
            // UPDATE temp_autodj_crates SET autodjrefs = autodjrefs - 1 WHERE track_id = :track_id;
            oQuery.prepare("UPDATE " AUTODJCRATES_TABLE " SET "
                AUTODJCRATESTABLE_AUTODJREFS " = " AUTODJCRATESTABLE_AUTODJREFS
                " - 1 WHERE " AUTODJCRATESTABLE_TRACKID " = :track_id");
            oQuery.bindValue(":track_id", iTrackId);
            if (!oQuery.exec()) {
                LOG_FAILED_QUERY(oQuery);
                return;
            }
            return;
        }
    }
}
// We are selecting the track in the following manner:
// We divide the library tracks into three sections, for which
// we sort the library according to times_played and select a
// percentage_of_prefered_tracks (70% by default ignoring the least-played
// 15% and most played 15%). We select a random track from this 70%
// or a random track from the remaining 30%.
//
// Example : For 100 random additions this will yield:
// 6 Tracks from first 15% (not favored)
// 88 from the middle 70% (favored)
// 6 from the last 15% (not favored)
// There for 88 favored tracks in 100 random addition.
// This is better than returning the least played track.
// Furthermore this also does not restrict our function to only retrieve
// not-played tracks (there is probably a reason they are not-played).

int AutoDJCratesDAO::getRandomTrackIdFromLibrary(const int iPlaylistId) {
    if(kLeastPreferredPercent >= 50 || kLeastPreferredPercent < 0){
        qDebug() << "Unacceptable value for kLeastPreferedPercent";
        return -1;
    }
    // getRandomTrackId() would have already created the temporary auto-DJ-crates database.
    QSqlQuery oQuery(m_rDatabase);
    // We ignore tracks from [0,ignoreIndex1] and [ignoreIndex2+1,most_played_Track]
    int iTrackId = -1, iTotalTracks = 0, beginIndex = 0, offset = 0, iIgnoreIndex1 = 0, iIgnoreIndex2 = 0;
    oQuery.prepare(" SELECT COUNT(*)"
                   " FROM library"
                   " WHERE id NOT IN"
                   " ( SELECT track_id "
                   " FROM PlaylistTracks"
                   " WHERE playlist_id = :id )"
                   " AND location NOT IN"
                   " ( SELECT id FROM track_locations"
                   " WHERE fs_deleted == 1 )"
                   " AND mixxx_deleted != 1" );
    oQuery.bindValue(":id",iPlaylistId);
    if (oQuery.exec()) {
        if (oQuery.next()) {
            iTotalTracks = oQuery.value(0).toInt();
        }
    } else {
        LOG_FAILED_QUERY(oQuery);
        return -1;
    }
    //qDebug() << "Total Tracks: "<<iTotalTracks;
    if(iTotalTracks == 0) return -1;

    if(kLeastPreferredPercent != 0){
        // Least Preferred is not disabled
        iIgnoreIndex1 = (kLeastPreferredPercent * iTotalTracks) / 100;
        iIgnoreIndex2 = iTotalTracks - iIgnoreIndex1;
        int iRandomNo = qrand() % 16 ;
        if(iRandomNo == 0 && iIgnoreIndex1 != 0) {
            // Select a track from the first [1, iIgnoredIndex1]
            beginIndex = 0;
            offset = qrand() % iIgnoreIndex1 + 1 ;
        } else if(iRandomNo == 1 && iTotalTracks > iIgnoreIndex2){
            // Select from [iIgnoredIndex2 + 1, iTotalTracks];
            beginIndex = iIgnoreIndex2;
            // We need a number between [1, Total - iIgnoreIndex2]
            offset = qrand() % (iTotalTracks - iIgnoreIndex2) + 1;
        } else {
            // Select from [iIgnoreIndex1 + 1, iIgnoreIndex2];
            beginIndex = iIgnoreIndex1;
            // We need a number between [1, iIgnoreIndex2 - iIgnoreIndex1]
            offset = qrand() % (iIgnoreIndex2 - iIgnoreIndex1) + 1;
        }
        offset = beginIndex + offset;
        // Incase we end up doing a qRand()%1 above
        if( offset >= iTotalTracks)
            offset= 0 ;
    }
    // Select tracks from library not in autoDJ playlist. Return track at the random offset
    oQuery.prepare(" SELECT id"
                   " FROM library"
                   " WHERE id NOT IN"
                   " ( SELECT track_id "
                   " FROM PlaylistTracks"
                   " WHERE playlist_id = :id )"
                   " AND location NOT IN"
                   " ( SELECT id FROM track_locations"
                   " WHERE fs_deleted == 1 )"
                   " AND mixxx_deleted != 1"
                   " ORDER BY timesplayed"
                   " LIMIT 1"
                   " OFFSET :offset");
    oQuery.bindValue(":id",iPlaylistId);
    oQuery.bindValue(":offset",offset);
    if (oQuery.exec()) {
        if (oQuery.next()) {
            //Get the trackId
            iTrackId = oQuery.value(0).toInt();
        }
    } else {
        LOG_FAILED_QUERY(oQuery);
    }
    return iTrackId;
}

