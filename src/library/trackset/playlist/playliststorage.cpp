#include "library/trackset/playlist/playliststorage.h"

#include "library/dao/playlistdao.h"
#include "library/dao/trackschema.h"
#include "util/db/fwdsqlquery.h"
#include "util/logger.h"

namespace {

const mixxx::Logger kLogger("PlaylistStorage");

const QString PLAYLISTTABLE_SORTNAME = "sort_name";

const QString PLAYLIST_SUMMARY_VIEW = "playlist_summary";

const QString PLAYLISTSUMMARY_TRACK_COUNT = "track_count";
const QString PLAYLISTSUMMARY_TRACK_DURATION = "track_duration";

const QString kPlaylistSummaryViewSelect =
        QStringLiteral(
                "SELECT"
                "  %1.id AS id, "
                "  %1.name AS name, "
                "  LOWER(%1.name) AS %4, "
                "  COUNT(case %3.%7 when 0 then 1 else null end) AS %8, "
                "  SUM(case %3.%7 when 0 then %3.duration else 0 end) AS %9 "
                "FROM %1 "
                "LEFT JOIN %2 ON %2.%5 = %1.id "
                "LEFT JOIN %3 ON %2.%6 = %3.id "
                "WHERE %1.hidden = 0 "
                "GROUP BY %1.id")
                .arg(
                        PLAYLIST_TABLE,
                        PLAYLIST_TRACKS_TABLE,
                        LIBRARY_TABLE,
                        PLAYLISTTABLE_SORTNAME,
                        PLAYLISTTRACKSTABLE_PLAYLISTID,
                        PLAYLISTTRACKSTABLE_TRACKID,
                        LIBRARYTABLE_MIXXXDELETED,
                        PLAYLISTSUMMARY_TRACK_COUNT,
                        PLAYLISTSUMMARY_TRACK_DURATION);

const QString kPlaylistSummaryViewQuery =
        QStringLiteral("CREATE TEMPORARY VIEW IF NOT EXISTS %1 AS %2")
                .arg(PLAYLIST_SUMMARY_VIEW, kPlaylistSummaryViewSelect);

} // namespace

PlaylistQueryFields::PlaylistQueryFields(const FwdSqlQuery& query)
        : m_iId(query.fieldIndex(PLAYLISTTABLE_ID)), m_iName(query.fieldIndex(PLAYLISTTABLE_NAME)) {
}

void PlaylistQueryFields::populateFromQuery(const FwdSqlQuery& query, Playlist* pPlaylist) const {
    pPlaylist->setId(getId(query));
    pPlaylist->setName(getName(query));
}

PlaylistSummaryQueryFields::PlaylistSummaryQueryFields(const FwdSqlQuery& query)
        : PlaylistQueryFields(query),
          m_iTrackCount(query.fieldIndex(PLAYLISTSUMMARY_TRACK_COUNT)),
          m_iTrackDuration(query.fieldIndex(PLAYLISTSUMMARY_TRACK_DURATION)) {
}

void PlaylistSummaryQueryFields::populateFromQuery(
        const FwdSqlQuery& query, PlaylistSummary* pPlaylistSummary) const {
    PlaylistQueryFields::populateFromQuery(query, pPlaylistSummary);
    pPlaylistSummary->setTrackCount(getTrackCount(query));
    pPlaylistSummary->setTrackDuration(getTrackDuration(query));
}

void PlaylistStorage::repairDatabase(const QSqlDatabase& database) {
}

void PlaylistStorage::connectDatabase(const QSqlDatabase& database) {
    m_database = database;
    createViews();
}

void PlaylistStorage::disconnectDatabase() {
    // Ensure that we don't use the current database connection
    // any longer.
    m_database = QSqlDatabase();
}

void PlaylistStorage::createViews() {
    VERIFY_OR_DEBUG_ASSERT(
            FwdSqlQuery(m_database, kPlaylistSummaryViewQuery).execPrepared()) {
        kLogger.critical()
                << "Failed to create database view for playlist summaries!";
    }
}

uint PlaylistStorage::countPlaylists() const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT COUNT(*) FROM %1")
                    .arg(PLAYLIST_TABLE));
    if (query.execPrepared() && query.next()) {
        uint result = query.fieldValue(0).toUInt();
        DEBUG_ASSERT(!query.next());
        return result;
    } else {
        return 0;
    }
}

PlaylistSummarySelectResult PlaylistStorage::selectPlaylistSummaries() const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT * FROM %1 ORDER BY %2")
                    .arg(PLAYLIST_SUMMARY_VIEW, PLAYLISTTABLE_SORTNAME));
    if (query.execPrepared()) {
        return PlaylistSummarySelectResult(std::move(query));
    } else {
        return PlaylistSummarySelectResult();
    }
}

bool PlaylistStorage::readPlaylistSummaryById(
        PlaylistId id, PlaylistSummary* pPlaylistSummary) const {
    FwdSqlQuery query(m_database,
            QStringLiteral(
                    "SELECT * FROM %1 WHERE %2=:id")
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
