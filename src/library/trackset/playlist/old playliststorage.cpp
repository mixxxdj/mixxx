#include "library/trackset/playlist/playliststorage.h"

#include "library/dao/trackschema.h"
#include "library/queryutil.h"
#include "library/trackset/playlist/playlist.h"
#include "library/trackset/playlist/playlistschema.h"
#include "library/trackset/playlist/playlistsummary.h"
#include "util/db/dbconnection.h"
#include "util/db/fwdsqlquery.h"
#include "util/db/sqllikewildcards.h"
#include "util/logger.h"

namespace {

const mixxx::Logger kLogger("PlaylistStorage");

const QString PLAYLISTTABLE_LOCKED = "locked";

const QString PLAYLIST_SUMMARY_VIEW = "playlist_summary";

const QString PLAYLISTSUMMARY_TRACK_COUNT = "track_count";
const QString PLAYLISTSUMMARY_TRACK_DURATION = "track_duration";

const QString kPlaylistTracksJoin =
        QStringLiteral("LEFT JOIN %3 ON %3.%4=%1.%2")
                .arg(PLAYLIST_TABLE,
                        PLAYLISTTABLE_ID,
                        PLAYLIST_TRACKS_TABLE,
                        PLAYLISTTRACKSTABLE_PLAYLISTID);

const QString kLibraryTracksJoin = kPlaylistTracksJoin +
        QStringLiteral(" LEFT JOIN %3 ON %3.%4=%1.%2")
                .arg(PLAYLIST_TRACKS_TABLE,
                        PLAYLISTTRACKSTABLE_TRACKID,
                        LIBRARY_TABLE,
                        LIBRARYTABLE_ID);

const QString kPlaylistSummaryViewSelect =
        QStringLiteral(
                "SELECT %1.*,"
                "COUNT(CASE %2.%4 WHEN 0 THEN 1 ELSE NULL END) AS %5,"
                "SUM(CASE %2.%4 WHEN 0 THEN %2.%3 ELSE 0 END) AS %6 "
                "FROM %1")
                .arg(
                        PLAYLIST_TABLE,
                        LIBRARY_TABLE,
                        LIBRARYTABLE_DURATION,
                        LIBRARYTABLE_MIXXXDELETED,
                        PLAYLISTSUMMARY_TRACK_COUNT,
                        PLAYLISTSUMMARY_TRACK_DURATION);

const QString kPlaylistSummaryViewQuery =
        QStringLiteral(
                "CREATE TEMPORARY VIEW IF NOT EXISTS %1 AS %2 %3 "
                "GROUP BY %4.%5")
                .arg(
                        PLAYLIST_SUMMARY_VIEW,
                        kPlaylistSummaryViewSelect,
                        kLibraryTracksJoin,
                        PLAYLIST_TABLE,
                        PLAYLISTTABLE_ID);

class PlaylistQueryBinder final {
  public:
    explicit PlaylistQueryBinder(FwdSqlQuery& query)
            : m_query(query) {
    }

    void bindId(const QString& placeholder, const Playlist& playlist) const {
        m_query.bindValue(placeholder, playlist.getId());
    }
    void bindName(const QString& placeholder, const Playlist& playlist) const {
        m_query.bindValue(placeholder, playlist.getName());
    }
    void bindLocked(const QString& placeholder, const Playlist& playlist) const {
        m_query.bindValue(placeholder, QVariant(playlist.isLocked()));
    }
    void bindAutoDjSource(const QString& placeholder, const Playlist& playlist) const {
        m_query.bindValue(placeholder, QVariant(playlist.isAutoDjSource()));
    }

  protected:
    FwdSqlQuery& m_query;
};

const QChar kSqlListSeparator(',');

// It is not possible to bind multiple values as a list to a query.
// The list of track ids has to be transformed into a single list
// string before it can be used in an SQL query.
QString joinSqlStringList(const QList<TrackId>& trackIds) {
    QString joinedTrackIds;
    // Reserve memory up front to prevent reallocation. Here we
    // assume that all track ids fit into 6 decimal digits and
    // add 1 character for the list separator.
    joinedTrackIds.reserve((6 + 1) * trackIds.size());
    for (const auto& trackId : trackIds) {
        if (!joinedTrackIds.isEmpty()) {
            joinedTrackIds += kSqlListSeparator;
        }
        joinedTrackIds += trackId.toString();
    }
    return joinedTrackIds;
}

} // anonymous namespace

PlaylistQueryFields::PlaylistQueryFields(const FwdSqlQuery& query)
        : m_iId(query.fieldIndex(PLAYLISTTABLE_ID)),
          m_iName(query.fieldIndex(PLAYLISTTABLE_NAME)),
          m_iLocked(query.fieldIndex(PLAYLISTTABLE_LOCKED)),
          m_iAutoDjSource(query.fieldIndex(PLAYLISTTABLE_AUTODJ_SOURCE)) {
}

void PlaylistQueryFields::populateFromQuery(
        const FwdSqlQuery& query,
        Playlist* pPlaylist) const {
    pPlaylist->setId(getId(query));
    pPlaylist->setName(getName(query));
    pPlaylist->setLocked(isLocked(query));
    pPlaylist->setAutoDjSource(isAutoDjSource(query));
}

PlaylistTrackQueryFields::PlaylistTrackQueryFields(const FwdSqlQuery& query)
        : m_iPlaylistId(query.fieldIndex(PLAYLISTTRACKSTABLE_PLAYLISTID)),
          m_iTrackId(query.fieldIndex(PLAYLISTTRACKSTABLE_TRACKID)) {
}

TrackQueryFields::TrackQueryFields(const FwdSqlQuery& query)
        : m_iTrackId(query.fieldIndex(PLAYLISTTRACKSTABLE_TRACKID)) {
}

PlaylistSummaryQueryFields::PlaylistSummaryQueryFields(const FwdSqlQuery& query)
        : PlaylistQueryFields(query),
          m_iTrackCount(query.fieldIndex(PLAYLISTSUMMARY_TRACK_COUNT)),
          m_iTrackDuration(query.fieldIndex(PLAYLISTSUMMARY_TRACK_DURATION)) {
}

void PlaylistSummaryQueryFields::populateFromQuery(
        const FwdSqlQuery& query,
        PlaylistSummary* pPlaylistSummary) const {
    PlaylistQueryFields::populateFromQuery(query, pPlaylistSummary);
    pPlaylistSummary->setTrackCount(getTrackCount(query));
    pPlaylistSummary->setTrackDuration(getTrackDuration(query));
}

void PlaylistStorage::repairDatabase(const QSqlDatabase& database) {
    // NOTE(uklotzde): No transactions
    // All queries are independent so there is no need to enclose some
    // or all of them in a transaction. Grouping into transactions would
    // improve the overall performance at the cost of increased resource
    // utilization. Since performance is not an issue for a maintenance
    // operation the decision was not to use any transactions.

    // NOTE(uklotzde): Nested scopes
    // Each of the following queries is enclosed in a nested scope.
    // When leaving this scope all resources allocated while executing
    // the query are released implicitly and before executing the next
    // query.

    // Playlists
    //    {
    //        // Delete playlists with empty names
    //        FwdSqlQuery query(database,
    //                QStringLiteral("DELETE FROM %1 WHERE %2 IS NULL OR TRIM(%2)=''")
    //                        .arg(PLAYLIST_TABLE, PLAYLISTTABLE_NAME));
    //        if (query.execPrepared() && (query.numRowsAffected() > 0)) {
    //            kLogger.warning()
    //                    << "Deleted" << query.numRowsAffected()
    //                    << "playlists with empty names";
    //        }
    //    }
    //    {
    //        // Fix invalid values in the "locked" column
    //        FwdSqlQuery query(database,
    //                QStringLiteral("UPDATE %1 SET %2=0 WHERE %2 NOT IN (0,1)")
    //                        .arg(PLAYLIST_TABLE, PLAYLISTTABLE_LOCKED));
    //        if (query.execPrepared() && (query.numRowsAffected() > 0)) {
    //            kLogger.warning()
    //                    << "Fixed boolean values in table" << PLAYLIST_TABLE
    //                    << "column" << PLAYLISTTABLE_LOCKED
    //                    << "for" << query.numRowsAffected() << "playlists";
    //        }
    //    }
    //    {
    //        // Fix invalid values in the "autodj_source" column
    //        FwdSqlQuery query(database,
    //                QStringLiteral("UPDATE %1 SET %2=0 WHERE %2 NOT IN (0,1)")
    //                        .arg(PLAYLIST_TABLE, PLAYLISTTABLE_AUTODJ_SOURCE));
    //        if (query.execPrepared() && (query.numRowsAffected() > 0)) {
    //            kLogger.warning()
    //                    << "Fixed boolean values in table" << PLAYLIST_TABLE
    //                    << "column" << PLAYLISTTABLE_AUTODJ_SOURCE
    //                    << "for" << query.numRowsAffected() << "playlists";
    //        }
    //    }
    //
    //    // Playlist tracks
    //    {
    //        // Remove tracks from non-existent playlists
    //        FwdSqlQuery query(database,
    //                QStringLiteral(
    //                        "DELETE FROM %1 WHERE %2 NOT IN (SELECT %3 FROM %4)")
    //                        .arg(PLAYLIST_TRACKS_TABLE,
    //                                PLAYLISTTRACKSTABLE_PLAYLISTID,
    //                                PLAYLISTTABLE_ID,
    //                                PLAYLIST_TABLE));
    //        if (query.execPrepared() && (query.numRowsAffected() > 0)) {
    //            kLogger.warning() << "Removed" << query.numRowsAffected()
    //                              << "playlist tracks from non-existent playlists";
    //        }
    //    }
    //    {
    //        // Remove library purged tracks from playlists
    //        FwdSqlQuery query(database,
    //                QStringLiteral(
    //                        "DELETE FROM %1 WHERE %2 NOT IN (SELECT %3 FROM %4)")
    //                        .arg(PLAYLIST_TRACKS_TABLE,
    //                                PLAYLISTTRACKSTABLE_TRACKID,
    //                                LIBRARYTABLE_ID,
    //                                LIBRARY_TABLE));
    //        if (query.execPrepared() && (query.numRowsAffected() > 0)) {
    //            kLogger.warning() << "Removed" << query.numRowsAffected()
    //                              << "library purged tracks from playlists";
    //        }
    //    }
}

void PlaylistStorage::connectDatabase(const QSqlDatabase& database) {
    //    m_database = database;
    //    createViews();
}

void PlaylistStorage::disconnectDatabase() {
    //    // Ensure that we don't use the current database connection
    //    // any longer.
    //    m_database = QSqlDatabase();
}

// void PlaylistStorage::createViews() {
//     VERIFY_OR_DEBUG_ASSERT(
//             FwdSqlQuery(m_database, kPlaylistSummaryViewQuery).execPrepared()) {
//         kLogger.critical()
//                 << "Failed to create database view for playlist summaries!";
//     }
// }

uint PlaylistStorage::countPlaylists() const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT COUNT(*) FROM %1").arg(PLAYLIST_TABLE));
    if (query.execPrepared() && query.next()) {
        uint result = query.fieldValue(0).toUInt();
        DEBUG_ASSERT(!query.next());
        return result;
    } else {
        return 0;
    }
}

bool PlaylistStorage::readPlaylistById(PlaylistId id, Playlist* pPlaylist) const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT * FROM %1 WHERE %2=:id")
                    .arg(PLAYLIST_TABLE, PLAYLISTTABLE_ID));
    query.bindValue(":id", id);
    if (query.execPrepared()) {
        PlaylistSelectResult playlists(std::move(query));
        if ((pPlaylist != nullptr) ? playlists.populateNext(pPlaylist) : playlists.next()) {
            VERIFY_OR_DEBUG_ASSERT(!playlists.next()) {
                kLogger.warning() << "Ambiguous playlist id:" << id;
            }
            return true;
        } else {
            kLogger.warning() << "Playlist not found by id:" << id;
        }
    }
    return false;
}

bool PlaylistStorage::readPlaylistByName(const QString& name, Playlist* pPlaylist) const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT * FROM %1 WHERE %2=:name")
                    .arg(PLAYLIST_TABLE, PLAYLISTTABLE_NAME));
    query.bindValue(":name", name);
    if (query.execPrepared()) {
        PlaylistSelectResult playlists(std::move(query));
        if ((pPlaylist != nullptr) ? playlists.populateNext(pPlaylist) : playlists.next()) {
            VERIFY_OR_DEBUG_ASSERT(!playlists.next()) {
                kLogger.warning() << "Ambiguous playlist name:" << name;
            }
            return true;
        } else {
            if (kLogger.debugEnabled()) {
                kLogger.debug() << "Playlist not found by name:" << name;
            }
        }
    }
    return false;
}

PlaylistSelectResult PlaylistStorage::selectPlaylists() const {
    FwdSqlQuery query(m_database,
            mixxx::DbConnection::collateLexicographically(
                    QStringLiteral("SELECT * FROM %1 ORDER BY %2")
                            .arg(PLAYLIST_TABLE, PLAYLISTTABLE_NAME)));

    if (query.execPrepared()) {
        return PlaylistSelectResult(std::move(query));
    } else {
        return PlaylistSelectResult();
    }
}

PlaylistSelectResult PlaylistStorage::selectPlaylistsByIds(
        const QString& subselectForPlaylistIds,
        SqlSubselectMode subselectMode) const {
    QString subselectPrefix;
    switch (subselectMode) {
    case SQL_SUBSELECT_IN:
        if (subselectForPlaylistIds.isEmpty()) {
            // edge case: no playlists
            return PlaylistSelectResult();
        }
        subselectPrefix = "IN";
        break;
    case SQL_SUBSELECT_NOT_IN:
        if (subselectForPlaylistIds.isEmpty()) {
            // edge case: all playlists
            return selectPlaylists();
        }
        subselectPrefix = "NOT IN";
        break;
    }
    DEBUG_ASSERT(!subselectPrefix.isEmpty());
    DEBUG_ASSERT(!subselectForPlaylistIds.isEmpty());

    FwdSqlQuery query(m_database,
            mixxx::DbConnection::collateLexicographically(
                    QStringLiteral("SELECT * FROM %1 "
                                   "WHERE %2 %3 (%4) "
                                   "ORDER BY %5")
                            .arg(PLAYLIST_TABLE,
                                    PLAYLISTTABLE_ID,
                                    subselectPrefix,
                                    subselectForPlaylistIds,
                                    PLAYLISTTABLE_NAME)));

    if (query.execPrepared()) {
        return PlaylistSelectResult(std::move(query));
    } else {
        return PlaylistSelectResult();
    }
}

PlaylistSelectResult PlaylistStorage::selectAutoDjPlaylists(bool autoDjSource) const {
    FwdSqlQuery query(m_database,
            mixxx::DbConnection::collateLexicographically(
                    QStringLiteral("SELECT * FROM %1 WHERE %2=:autoDjSource "
                                   "ORDER BY %3")
                            .arg(PLAYLIST_TABLE,
                                    PLAYLISTTABLE_AUTODJ_SOURCE,
                                    PLAYLISTTABLE_NAME)));
    query.bindValue(":autoDjSource", QVariant(autoDjSource));
    if (query.execPrepared()) {
        return PlaylistSelectResult(std::move(query));
    } else {
        return PlaylistSelectResult();
    }
}

PlaylistSummarySelectResult PlaylistStorage::selectPlaylistSummaries() const {
    FwdSqlQuery query(m_database,
            mixxx::DbConnection::collateLexicographically(
                    QStringLiteral("SELECT * FROM %1 ORDER BY %2")
                            .arg(PLAYLIST_SUMMARY_VIEW, PLAYLISTTABLE_NAME)));
    if (query.execPrepared()) {
        return PlaylistSummarySelectResult(std::move(query));
    } else {
        return PlaylistSummarySelectResult();
    }
}

bool PlaylistStorage::readPlaylistSummaryById(
        PlaylistId id, PlaylistSummary* pPlaylistSummary) const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT * FROM %1 WHERE %2=:id")
                    .arg(PLAYLIST_SUMMARY_VIEW, PLAYLISTTABLE_ID));
    query.bindValue(":id", id);
    if (query.execPrepared()) {
        PlaylistSummarySelectResult playlistSummaries(std::move(query));
        if ((pPlaylistSummary != nullptr)
                        ? playlistSummaries.populateNext(pPlaylistSummary)
                        : playlistSummaries.next()) {
            VERIFY_OR_DEBUG_ASSERT(!playlistSummaries.next()) {
                kLogger.warning() << "Ambiguous playlist id:" << id;
            }
            return true;
        } else {
            kLogger.warning() << "Playlist summary not found by id:" << id;
        }
    }
    return false;
}

uint PlaylistStorage::countPlaylistTracks(PlaylistId playlistId) const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT COUNT(*) FROM %1 WHERE %2=:playlistId")
                    .arg(PLAYLIST_TRACKS_TABLE, PLAYLISTTRACKSTABLE_PLAYLISTID));
    query.bindValue(":playlistId", playlistId);
    if (query.execPrepared() && query.next()) {
        uint result = query.fieldValue(0).toUInt();
        DEBUG_ASSERT(!query.next());
        return result;
    } else {
        return 0;
    }
}

// static
QString PlaylistStorage::formatSubselectQueryForPlaylistTrackIds(PlaylistId playlistId) {
    return QStringLiteral("SELECT %1 FROM %2 WHERE %3=%4")
            .arg(PLAYLISTTRACKSTABLE_TRACKID,
                    PLAYLIST_TRACKS_TABLE,
                    PLAYLISTTRACKSTABLE_PLAYLISTID,
                    playlistId.toString());
}

QString PlaylistStorage::formatQueryForTrackIdsByPlaylistNameLike(
        const QString& playlistNameLike) const {
    FieldEscaper escaper(m_database);
    QString escapedPlaylistNameLike = escaper.escapeString(
            kSqlLikeMatchAll + playlistNameLike + kSqlLikeMatchAll);
    return QString(
            "SELECT DISTINCT %1 FROM %2 "
            "JOIN %3 ON %4=%5 WHERE %6 LIKE %7 "
            "ORDER BY %1")
            .arg(PLAYLISTTRACKSTABLE_TRACKID,
                    PLAYLIST_TRACKS_TABLE,
                    PLAYLIST_TABLE,
                    PLAYLISTTRACKSTABLE_PLAYLISTID,
                    PLAYLISTTABLE_ID,
                    PLAYLISTTABLE_NAME,
                    escapedPlaylistNameLike);
}

// static
QString PlaylistStorage::formatQueryForTrackIdsWithPlaylist() {
    return QStringLiteral(
            "SELECT DISTINCT %1 FROM %2 JOIN %3 ON %4=%5 ORDER BY %1")
            .arg(PLAYLISTTRACKSTABLE_TRACKID,
                    PLAYLIST_TRACKS_TABLE,
                    PLAYLIST_TABLE,
                    PLAYLISTTRACKSTABLE_PLAYLISTID,
                    PLAYLISTTABLE_ID);
}

PlaylistTrackSelectResult PlaylistStorage::selectPlaylistTracksSorted(
        PlaylistId playlistId) const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT * FROM %1 WHERE %2=:playlistId ORDER BY %3")
                    .arg(PLAYLIST_TRACKS_TABLE,
                            PLAYLISTTRACKSTABLE_PLAYLISTID,
                            PLAYLISTTRACKSTABLE_TRACKID));
    query.bindValue(":playlistId", playlistId);
    if (query.execPrepared()) {
        return PlaylistTrackSelectResult(std::move(query));
    } else {
        return PlaylistTrackSelectResult();
    }
}

PlaylistTrackSelectResult PlaylistStorage::selectTrackPlaylistsSorted(
        TrackId trackId) const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT * FROM %1 WHERE %2=:trackId ORDER BY %3")
                    .arg(PLAYLIST_TRACKS_TABLE,
                            PLAYLISTTRACKSTABLE_TRACKID,
                            PLAYLISTTRACKSTABLE_PLAYLISTID));
    query.bindValue(":trackId", trackId);
    if (query.execPrepared()) {
        return PlaylistTrackSelectResult(std::move(query));
    } else {
        return PlaylistTrackSelectResult();
    }
}

PlaylistSummarySelectResult PlaylistStorage::selectPlaylistsWithTrackCount(
        const QList<TrackId>& trackIds) const {
    FwdSqlQuery query(m_database,
            mixxx::DbConnection::collateLexicographically(
                    QStringLiteral("SELECT *, "
                                   "(SELECT COUNT(*) FROM %1 WHERE %2.%3 = %1.%4 and "
                                   "%1.%5 in (%9)) AS %6, "
                                   "0 as %7 FROM %2 ORDER BY %8")
                            .arg(
                                    PLAYLIST_TRACKS_TABLE,
                                    PLAYLIST_TABLE,
                                    PLAYLISTTABLE_ID,
                                    PLAYLISTTRACKSTABLE_PLAYLISTID,
                                    PLAYLISTTRACKSTABLE_TRACKID,
                                    PLAYLISTSUMMARY_TRACK_COUNT,
                                    PLAYLISTSUMMARY_TRACK_DURATION,
                                    PLAYLISTTABLE_NAME,
                                    joinSqlStringList(trackIds))));

    if (query.execPrepared()) {
        return PlaylistSummarySelectResult(std::move(query));
    } else {
        return PlaylistSummarySelectResult();
    }
}

PlaylistTrackSelectResult PlaylistStorage::selectTracksSortedByPlaylistNameLike(
        const QString& playlistNameLike) const {
    // TODO: Do SQL LIKE wildcards in playlistNameLike need to be escaped?
    // Previously we used SqlLikeWildcardEscaper in the past for this
    // purpose. This utility class has become obsolete but could be
    // restored from the 2.3 branch if ever needed again.
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT %1,%2 FROM %3 "
                           "JOIN %4 ON %5 = %6 "
                           "WHERE %7 LIKE :playlistNameLike "
                           "ORDER BY %1")
                    .arg(PLAYLISTTRACKSTABLE_TRACKID,
                            PLAYLISTTRACKSTABLE_PLAYLISTID,
                            PLAYLIST_TRACKS_TABLE,
                            PLAYLIST_TABLE,
                            PLAYLISTTABLE_ID,
                            PLAYLISTTRACKSTABLE_PLAYLISTID,
                            PLAYLISTTABLE_NAME));
    query.bindValue(":playlistNameLike",
            QVariant(kSqlLikeMatchAll + playlistNameLike + kSqlLikeMatchAll));

    if (query.execPrepared()) {
        return PlaylistTrackSelectResult(std::move(query));
    } else {
        return PlaylistTrackSelectResult();
    }
}

TrackSelectResult PlaylistStorage::selectAllTracksSorted() const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT DISTINCT %1 FROM %2 ORDER BY %1")
                    .arg(PLAYLISTTRACKSTABLE_TRACKID, PLAYLIST_TRACKS_TABLE));
    if (query.execPrepared()) {
        return TrackSelectResult(std::move(query));
    } else {
        return TrackSelectResult();
    }
}

QSet<PlaylistId> PlaylistStorage::collectPlaylistIdsOfTracks(const QList<TrackId>& trackIds) const {
    // NOTE(uklotzde): One query per track id. This could be optimized
    // by querying for chunks of track ids and collecting the results.
    QSet<PlaylistId> trackPlaylists;
    for (const auto& trackId : trackIds) {
        // NOTE(uklotzde): The query result does not need to be sorted by playlist id
        // here. But since the corresponding FK column is indexed the impact on the
        // performance should be negligible. By reusing an existing query we reduce
        // the amount of code and the number of prepared SQL queries.
        PlaylistTrackSelectResult playlistTracks(selectTrackPlaylistsSorted(trackId));
        while (playlistTracks.next()) {
            DEBUG_ASSERT(playlistTracks.trackId() == trackId);
            trackPlaylists.insert(playlistTracks.playlistId());
        }
    }
    return trackPlaylists;
}

bool PlaylistStorage::onInsertingPlaylist(
        const Playlist& playlist,
        PlaylistId* pPlaylistId) {
    VERIFY_OR_DEBUG_ASSERT(!playlist.getId().isValid()) {
        kLogger.warning()
                << "Cannot insert playlist with a valid id:" << playlist.getId();
        return false;
    }
    FwdSqlQuery query(m_database,
            QStringLiteral(
                    "INSERT INTO %1 (%2,%3,%4) "
                    "VALUES (:name,:locked,:autoDjSource)")
                    .arg(
                            PLAYLIST_TABLE,
                            PLAYLISTTABLE_NAME,
                            PLAYLISTTABLE_LOCKED,
                            PLAYLISTTABLE_AUTODJ_SOURCE));
    VERIFY_OR_DEBUG_ASSERT(query.isPrepared()) {
        return false;
    }
    PlaylistQueryBinder queryBinder(query);
    queryBinder.bindName(":name", playlist);
    queryBinder.bindLocked(":locked", playlist);
    queryBinder.bindAutoDjSource(":autoDjSource", playlist);
    VERIFY_OR_DEBUG_ASSERT(query.execPrepared()) {
        return false;
    }
    if (query.numRowsAffected() > 0) {
        DEBUG_ASSERT(query.numRowsAffected() == 1);
        if (pPlaylistId != nullptr) {
            *pPlaylistId = PlaylistId(query.lastInsertId());
            DEBUG_ASSERT(pPlaylistId->isValid());
        }
        return true;
    } else {
        return false;
    }
}

bool PlaylistStorage::onUpdatingPlaylist(
        const Playlist& playlist) {
    VERIFY_OR_DEBUG_ASSERT(playlist.getId().isValid()) {
        kLogger.warning()
                << "Cannot update playlist without a valid id";
        return false;
    }
    FwdSqlQuery query(m_database,
            QString(
                    "UPDATE %1 "
                    "SET %2=:name,%3=:locked,%4=:autoDjSource "
                    "WHERE %5=:id")
                    .arg(
                            PLAYLIST_TABLE,
                            PLAYLISTTABLE_NAME,
                            PLAYLISTTABLE_LOCKED,
                            PLAYLISTTABLE_AUTODJ_SOURCE,
                            PLAYLISTTABLE_ID));
    VERIFY_OR_DEBUG_ASSERT(query.isPrepared()) {
        return false;
    }
    PlaylistQueryBinder queryBinder(query);
    queryBinder.bindId(":id", playlist);
    queryBinder.bindName(":name", playlist);
    queryBinder.bindLocked(":locked", playlist);
    queryBinder.bindAutoDjSource(":autoDjSource", playlist);
    VERIFY_OR_DEBUG_ASSERT(query.execPrepared()) {
        return false;
    }
    if (query.numRowsAffected() > 0) {
        VERIFY_OR_DEBUG_ASSERT(query.numRowsAffected() <= 1) {
            kLogger.warning()
                    << "Updated multiple playlists with the same id" << playlist.getId();
        }
        return true;
    } else {
        kLogger.warning()
                << "Cannot update non-existent playlist with id" << playlist.getId();
        return false;
    }
}

bool PlaylistStorage::onDeletingPlaylist(
        PlaylistId playlistId) {
    VERIFY_OR_DEBUG_ASSERT(playlistId.isValid()) {
        kLogger.warning()
                << "Cannot delete playlist without a valid id";
        return false;
    }
    {
        FwdSqlQuery query(m_database,
                QStringLiteral("DELETE FROM %1 WHERE %2=:id")
                        .arg(PLAYLIST_TRACKS_TABLE, PLAYLISTTRACKSTABLE_PLAYLISTID));
        VERIFY_OR_DEBUG_ASSERT(query.isPrepared()) {
            return false;
        }
        query.bindValue(":id", playlistId);
        VERIFY_OR_DEBUG_ASSERT(query.execPrepared()) {
            return false;
        }
        if (query.numRowsAffected() <= 0) {
            if (kLogger.debugEnabled()) {
                kLogger.debug()
                        << "Deleting empty playlist with id"
                        << playlistId;
            }
        }
    }
    {
        FwdSqlQuery query(m_database,
                QStringLiteral("DELETE FROM %1 WHERE %2=:id")
                        .arg(PLAYLIST_TABLE, PLAYLISTTABLE_ID));
        VERIFY_OR_DEBUG_ASSERT(query.isPrepared()) {
            return false;
        }
        query.bindValue(":id", playlistId);
        VERIFY_OR_DEBUG_ASSERT(query.execPrepared()) {
            return false;
        }
        if (query.numRowsAffected() > 0) {
            VERIFY_OR_DEBUG_ASSERT(query.numRowsAffected() <= 1) {
                kLogger.warning()
                        << "Deleted multiple playlists with the same id" << playlistId;
            }
            return true;
        } else {
            kLogger.warning()
                    << "Cannot delete non-existent playlist with id" << playlistId;
            return false;
        }
    }
}

bool PlaylistStorage::onAddingPlaylistTracks(
        PlaylistId playlistId,
        const QList<TrackId>& trackIds) {
    FwdSqlQuery query(m_database,
            QStringLiteral(
                    "INSERT OR IGNORE INTO %1 (%2, %3) "
                    "VALUES (:playlistId,:trackId)")
                    .arg(
                            PLAYLIST_TRACKS_TABLE,
                            PLAYLISTTRACKSTABLE_PLAYLISTID,
                            PLAYLISTTRACKSTABLE_TRACKID));
    if (!query.isPrepared()) {
        return false;
    }
    query.bindValue(":playlistId", playlistId);
    for (const auto& trackId : trackIds) {
        query.bindValue(":trackId", trackId);
        if (!query.execPrepared()) {
            return false;
        }
        if (query.numRowsAffected() == 0) {
            // track is already in playlist
            if (kLogger.debugEnabled()) {
                kLogger.debug()
                        << "Track" << trackId
                        << "not added to playlist" << playlistId;
            }
        } else {
            DEBUG_ASSERT(query.numRowsAffected() == 1);
        }
    }
    return true;
}

bool PlaylistStorage::onRemovingPlaylistTracks(
        PlaylistId playlistId,
        const QList<TrackId>& trackIds) {
    // NOTE(uklotzde): We remove tracks in a loop
    // analogously to adding tracks (see above).
    FwdSqlQuery query(m_database,
            QStringLiteral(
                    "DELETE FROM %1 "
                    "WHERE %2=:playlistId AND %3=:trackId")
                    .arg(
                            PLAYLIST_TRACKS_TABLE,
                            PLAYLISTTRACKSTABLE_PLAYLISTID,
                            PLAYLISTTRACKSTABLE_TRACKID));
    if (!query.isPrepared()) {
        return false;
    }
    query.bindValue(":playlistId", playlistId);
    for (const auto& trackId : trackIds) {
        query.bindValue(":trackId", trackId);
        if (!query.execPrepared()) {
            return false;
        }
        if (query.numRowsAffected() == 0) {
            // track not found in playlist
            if (kLogger.debugEnabled()) {
                kLogger.debug()
                        << "Track" << trackId
                        << "not removed from playlist" << playlistId;
            }
        } else {
            DEBUG_ASSERT(query.numRowsAffected() == 1);
        }
    }
    return true;
}

bool PlaylistStorage::onPurgingTracks(
        const QList<TrackId>& trackIds) {
    // NOTE(uklotzde): Remove tracks from playlists one-by-one.
    // This might be optimized by deleting multiple track ids
    // at once in chunks with a maximum size.
    FwdSqlQuery query(m_database,
            QStringLiteral("DELETE FROM %1 WHERE %2=:trackId")
                    .arg(PLAYLIST_TRACKS_TABLE, PLAYLISTTRACKSTABLE_TRACKID));
    if (!query.isPrepared()) {
        return false;
    }
    for (const auto& trackId : trackIds) {
        query.bindValue(":trackId", trackId);
        if (!query.execPrepared()) {
            return false;
        }
    }
    return true;
}
