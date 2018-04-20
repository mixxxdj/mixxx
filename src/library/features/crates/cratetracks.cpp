#include "library/queryutil.h"
#include "util/db/sqllikewildcards.h"
#include "library/features/crates/cratetracks.h"
#include "util/logger.h"

namespace {

const mixxx::Logger kLogger("CrateStorage");

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
    for (const auto& trackId: trackIds) {
        if (!joinedTrackIds.isEmpty()) {
            joinedTrackIds += kSqlListSeparator;
        }
        joinedTrackIds += trackId.toString();
    }
    return joinedTrackIds;
}
} // anonymus namespace

void CrateTracks::initialize(const QSqlDatabase& database) {
     m_database = database;
}

bool CrateTracks::onAddingCrateTracks(
        CrateId crateId,
        const QList<TrackId>& trackIds) {
    FwdSqlQuery query(m_database, QString(
            "INSERT OR IGNORE INTO %1 (%2, %3) VALUES (:crateId,:trackId)").arg(
                    CRATE_TRACKS_TABLE,
                    CRATETRACKSTABLE_CRATEID,
                    CRATETRACKSTABLE_TRACKID));
    if (!query.isPrepared()) {
        return false;
    }
    query.bindValue(":crateId", crateId);
    for (const auto& trackId: trackIds) {
        query.bindValue(":trackId", trackId);
        if (!query.execPrepared()) {
            return false;
        }
        if (query.numRowsAffected() == 0) {
            // track is already in crate
            kLogger.debug()
                    << "Track" << trackId
                    << "not added to crate" << crateId;
        } else {
            DEBUG_ASSERT(query.numRowsAffected() == 1);
        }
    }
    return true;
}


bool CrateTracks::onRemovingCrateTracks(
        CrateId crateId,
        const QList<TrackId>& trackIds) {
    // NOTE(uklotzde): We remove tracks in a loop
    // analogously to adding tracks (see above).
    FwdSqlQuery query(m_database, QString(
            "DELETE FROM %1 WHERE %2=:crateId AND %3=:trackId").arg(
                    CRATE_TRACKS_TABLE,
                    CRATETRACKSTABLE_CRATEID,
                    CRATETRACKSTABLE_TRACKID));
    if (!query.isPrepared()) {
        return false;
    }
    query.bindValue(":crateId", crateId);
    for (const auto& trackId: trackIds) {
        query.bindValue(":trackId", trackId);
        if (!query.execPrepared()) {
            return false;
        }
        if (query.numRowsAffected() == 0) {
            // track not found in crate
            kLogger.debug()
                    << "Track" << trackId
                    << "not removed from crate" << crateId;
        } else {
            DEBUG_ASSERT(query.numRowsAffected() == 1);
        }
    }
    return true;
}


bool CrateTracks::onPurgingTracks(
        const QList<TrackId>& trackIds) {
    // NOTE(uklotzde): Remove tracks from crates one-by-one.
    // This might be optimized by deleting multiple track ids
    // at once in chunks with a maximum size.
    FwdSqlQuery query(m_database, QString(
            "DELETE FROM %1 WHERE %2=:trackId").arg(
                    CRATE_TRACKS_TABLE,
                    CRATETRACKSTABLE_TRACKID));
    if (!query.isPrepared()) {
        return false;
    }
    for (const auto& trackId: trackIds) {
        query.bindValue(":trackId", trackId);
        if (!query.execPrepared()) {
            return false;
        }
    }
    return true;
}


uint CrateTracks::countCrateTracks(CrateId crateId) const {
    FwdSqlQuery query(m_database, QString(
            "SELECT COUNT(*) FROM %1 WHERE %2=:crateId").arg(
                    CRATE_TRACKS_TABLE,
                    CRATETRACKSTABLE_CRATEID));
    query.bindValue(":crateId", crateId);
    if (query.execPrepared() && query.next()) {
        uint result = query.fieldValue(0).toUInt();
        DEBUG_ASSERT(!query.next());
        return result;
    } else {
        return 0;
    }
}

//static
QString CrateTracks::formatSubselectQueryForCrateTrackIds(
        CrateId crateId) {
    return QString("SELECT %1 FROM %2 WHERE %3=%4").arg(
            CRATETRACKSTABLE_TRACKID,
            CRATE_TRACKS_TABLE,
            CRATETRACKSTABLE_CRATEID,
            crateId.toString());
}

CrateTrackSelectResult CrateTracks::selectCrateTracksSorted(CrateId crateId) const {
    FwdSqlQuery query(m_database, QString(
            "SELECT * FROM %1 WHERE %2=:crateId ORDER BY %3").arg(
                    CRATE_TRACKS_TABLE,
                    CRATETRACKSTABLE_CRATEID,
                    CRATETRACKSTABLE_TRACKID));
    query.bindValue(":crateId", crateId);
    if (query.execPrepared()) {
        return CrateTrackSelectResult(std::move(query));
    } else {
        return CrateTrackSelectResult();
    }
}

CrateTrackSelectResult CrateTracks::selectTrackCratesSorted(TrackId trackId) const {
    FwdSqlQuery query(m_database, QString(
            "SELECT * FROM %1 WHERE %2=:trackId ORDER BY %3").arg(
                    CRATE_TRACKS_TABLE,
                    CRATETRACKSTABLE_TRACKID,
                    CRATETRACKSTABLE_CRATEID));
    query.bindValue(":trackId", trackId);
    if (query.execPrepared()) {
        return CrateTrackSelectResult(std::move(query));
    } else {
        return CrateTrackSelectResult();
    }
}

CrateTrackSelectResult CrateTracks::selectTracksSortedByCrateNameLike(const QString& crateNameLike) const {
    FwdSqlQuery query(m_database, QString(
            "SELECT %1,%2 FROM %3 JOIN %4 ON %5 = %6 WHERE %7 LIKE :crateNameLike ORDER BY %1").arg(
                    CRATETRACKSTABLE_TRACKID,
                    CRATETRACKSTABLE_CRATEID,
                    CRATE_TRACKS_TABLE,
                    CRATE_TABLE,
                    CRATETABLE_ID,
                    CRATETRACKSTABLE_CRATEID,
                    CRATETABLE_NAME));
    query.bindValue(":crateNameLike", kSqlLikeMatchAll + crateNameLike + kSqlLikeMatchAll);

    if (query.execPrepared()) {
        return CrateTrackSelectResult(std::move(query));
    } else {
        return CrateTrackSelectResult();
    }
}

CrateSummarySelectResult CrateTracks::selectCratesWithTrackCount(const QList<TrackId>& trackIds) const {
    FwdSqlQuery query(m_database, QString(
        "SELECT *, ("
        "    SELECT COUNT(*) FROM %1 WHERE %2.%3 = %1.%4 and %1.%5 in (%9)"
        " ) AS %6, 0 as %7 FROM %2 ORDER BY %8").arg(
                CRATE_TRACKS_TABLE,
                CRATE_TABLE,
                CRATETABLE_ID,
                CRATETRACKSTABLE_CRATEID,
                CRATETRACKSTABLE_TRACKID,
                CRATESUMMARY_TRACK_COUNT,
                CRATESUMMARY_TRACK_DURATION,
                CRATETABLE_NAME,
                joinSqlStringList(trackIds)));

    if (query.execPrepared()) {
        return CrateSummarySelectResult(std::move(query));
    } else {
        return CrateSummarySelectResult();
    }
}


QSet<CrateId> CrateTracks::collectCrateIdsOfTracks(const QList<TrackId>& trackIds) const {
    // NOTE(uklotzde): One query per track id. This could be optimized
    // by querying for chunks of track ids and collecting the results.
    QSet<CrateId> trackCrates;
    for (const auto& trackId: trackIds) {
        // NOTE(uklotzde): The query result does not need to be sorted by crate id
        // here. But since the coresponding FK column is indexed the impact on the
        // performance should be negligible. By reusing an existing query we reduce
        // the amount of code and the number of prepared SQL queries.
        CrateTrackSelectResult crateTracks(selectTrackCratesSorted(trackId));
        while (crateTracks.next()) {
            DEBUG_ASSERT(crateTracks.trackId() == trackId);
            trackCrates.insert(crateTracks.crateId());
        }
    }
    return trackCrates;
}
