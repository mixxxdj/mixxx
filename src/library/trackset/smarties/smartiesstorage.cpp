#include "library/trackset/smarties/smartiesstorage.h"

#include "library/dao/trackschema.h"
#include "library/queryutil.h"
#include "library/trackset/smarties/smarties.h"
#include "library/trackset/smarties/smartiesschema.h"
#include "library/trackset/smarties/smartiessummary.h"
#include "util/db/dbconnection.h"
#include "util/db/fwdsqlquery.h"
#include "util/db/sqllikewildcards.h"
#include "util/logger.h"

namespace {

const mixxx::Logger kLogger("SmartiesStorage");

const QString SMARTIESTABLE_LOCKED = "locked";

const QString SMARTIES_SUMMARY_VIEW = "smarties_summary";

const QString SMARTIESSUMMARY_TRACK_COUNT = "track_count";
const QString SMARTIESSUMMARY_TRACK_DURATION = "track_duration";

const QString kSmartiesTracksJoin =
        QStringLiteral("LEFT JOIN %3 ON %3.%4=%1.%2")
                .arg(SMARTIES_TABLE,
                        SMARTIESTABLE_ID,
                        SMARTIES_TRACKS_TABLE,
                        SMARTIESTRACKSTABLE_SMARTIESID);

const QString kLibraryTracksJoin = kSmartiesTracksJoin +
        QStringLiteral(" LEFT JOIN %3 ON %3.%4=%1.%2")
                .arg(SMARTIES_TRACKS_TABLE,
                        SMARTIESTRACKSTABLE_TRACKID,
                        LIBRARY_TABLE,
                        LIBRARYTABLE_ID);

const QString kSmartiesSummaryViewSelect =
        QStringLiteral(
                "SELECT %1.*,"
                "COUNT(CASE %2.%4 WHEN 0 THEN 1 ELSE NULL END) AS %5,"
                "SUM(CASE %2.%4 WHEN 0 THEN %2.%3 ELSE 0 END) AS %6 "
                "FROM %1")
                .arg(
                        SMARTIES_TABLE,
                        LIBRARY_TABLE,
                        LIBRARYTABLE_DURATION,
                        LIBRARYTABLE_MIXXXDELETED,
                        SMARTIESSUMMARY_TRACK_COUNT,
                        SMARTIESSUMMARY_TRACK_DURATION);

const QString kSmartiesSummaryViewQuery =
        QStringLiteral(
                "CREATE TEMPORARY VIEW IF NOT EXISTS %1 AS %2 %3 "
                "GROUP BY %4.%5")
                .arg(
                        SMARTIES_SUMMARY_VIEW,
                        kSmartiesSummaryViewSelect,
                        kLibraryTracksJoin,
                        SMARTIES_TABLE,
                        SMARTIESTABLE_ID);

class SmartiesQueryBinder final {
  public:
    explicit SmartiesQueryBinder(FwdSqlQuery& query)
            : m_query(query) {
    }

    void bindId(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, smarties.getId());
    }
    void bindName(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, smarties.getName());
    }
    void bindLocked(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.isLocked()));
    }
    void bindAutoDjSource(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.isAutoDjSource()));
    }

    void bindSearchInput(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getSearchInput()));
    }
    void bindSearchSql(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getSearchSql()));
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

SmartiesQueryFields::SmartiesQueryFields(const FwdSqlQuery& query)
        : m_iId(query.fieldIndex(SMARTIESTABLE_ID)),
          m_iName(query.fieldIndex(SMARTIESTABLE_NAME)),
          m_iLocked(query.fieldIndex(SMARTIESTABLE_LOCKED)),
          m_iAutoDjSource(query.fieldIndex(SMARTIESTABLE_AUTODJ_SOURCE)),
          m_iSearchInput(query.fieldIndex(SMARTIESTABLE_SEARCH_INPUT)),
          m_iSearchSql(query.fieldIndex(SMARTIESTABLE_SEARCH_SQL)) {
}

void SmartiesQueryFields::populateFromQuery(
        const FwdSqlQuery& query,
        Smarties* pSmarties) const {
    pSmarties->setId(getId(query));
    pSmarties->setName(getName(query));
    pSmarties->setLocked(isLocked(query));
    pSmarties->setAutoDjSource(isAutoDjSource(query));
    pSmarties->setSearchInput(getSearchInput(query));
    pSmarties->setSearchSql(getSearchSql(query));
}

SmartiesTrackQueryFields::SmartiesTrackQueryFields(const FwdSqlQuery& query)
        : m_iSmartiesId(query.fieldIndex(SMARTIESTRACKSTABLE_SMARTIESID)),
          m_iTrackId(query.fieldIndex(SMARTIESTRACKSTABLE_TRACKID)) {
}

// TrackQueryFields::TrackQueryFields(const FwdSqlQuery& query)
//         : m_iTrackId(query.fieldIndex(SMARTIESTRACKSTABLE_TRACKID)) {
// }

SmartiesSummaryQueryFields::SmartiesSummaryQueryFields(const FwdSqlQuery& query)
        : SmartiesQueryFields(query),
          m_iTrackCount(query.fieldIndex(SMARTIESSUMMARY_TRACK_COUNT)),
          m_iTrackDuration(query.fieldIndex(SMARTIESSUMMARY_TRACK_DURATION)) {
}

void SmartiesSummaryQueryFields::populateFromQuery(
        const FwdSqlQuery& query,
        SmartiesSummary* pSmartiesSummary) const {
    SmartiesQueryFields::populateFromQuery(query, pSmartiesSummary);
    pSmartiesSummary->setTrackCount(getTrackCount(query));
    pSmartiesSummary->setTrackDuration(getTrackDuration(query));
}

void SmartiesStorage::repairDatabase(const QSqlDatabase& database) {
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

    // Smarties
    {
        // Delete smarties with empty names
        FwdSqlQuery query(database,
                QStringLiteral("DELETE FROM %1 WHERE %2 IS NULL OR TRIM(%2)=''")
                        .arg(SMARTIES_TABLE, SMARTIESTABLE_NAME));
        if (query.execPrepared() && (query.numRowsAffected() > 0)) {
            kLogger.warning()
                    << "Deleted" << query.numRowsAffected()
                    << "smarties with empty names";
        }
    }
    {
        // Delete smarties with empty search_input
        FwdSqlQuery query(database,
                QStringLiteral("DELETE FROM %1 WHERE %2 IS NULL")
                        .arg(SMARTIES_TABLE, SMARTIESTABLE_SEARCH_INPUT));
        if (query.execPrepared() && (query.numRowsAffected() > 0)) {
            kLogger.warning()
                    << "Deleted" << query.numRowsAffected()
                    << "smarties with empty search_input";
        }
    }
    //    {
    // Delete smarties with empty search_sql
    //        FwdSqlQuery query(database,
    //                QStringLiteral("DELETE FROM %1 WHERE %2 IS NULL")
    //                        .arg(SMARTIES_TABLE, SMARTIESTABLE_SEARCH_SQL));
    //        if (query.execPrepared() && (query.numRowsAffected() > 0)) {
    //            kLogger.warning()
    //                    << "Deleted" << query.numRowsAffected()
    //                    << "smarties with empty search_sql";
    //        }
    //    }
    //    {
    //        // Fix invalid values in the "locked" column
    //        FwdSqlQuery query(database,
    //                QStringLiteral("UPDATE %1 SET %2=0 WHERE %2 NOT IN (0,1)")
    //                        .arg(SMARTIES_TABLE, SMARTIESTABLE_LOCKED));
    //        if (query.execPrepared() && (query.numRowsAffected() > 0)) {
    //            kLogger.warning()
    //                    << "Fixed boolean values in table" << SMARTIES_TABLE
    //                    << "column" << SMARTIESTABLE_LOCKED
    //                    << "for" << query.numRowsAffected() << "smarties";
    //        }
    //    }
    //    {
    //        // Fix invalid values in the "autodj_source" column
    //        FwdSqlQuery query(database,
    //                QStringLiteral("UPDATE %1 SET %2=0 WHERE %2 NOT IN (0,1)")
    //                        .arg(SMARTIES_TABLE, SMARTIESTABLE_AUTODJ_SOURCE));
    //        if (query.execPrepared() && (query.numRowsAffected() > 0)) {
    //            kLogger.warning()
    //                    << "Fixed boolean values in table" << SMARTIES_TABLE
    //                    << "column" << SMARTIESTABLE_AUTODJ_SOURCE
    //                    << "for" << query.numRowsAffected() << "smarties";
    //        }
    //    }

    // Smarties tracks
    {
        // Remove tracks from non-existent smarties
        FwdSqlQuery query(database,
                QStringLiteral(
                        "DELETE FROM %1 WHERE %2 NOT IN (SELECT %3 FROM %4)")
                        .arg(SMARTIES_TRACKS_TABLE,
                                SMARTIESTRACKSTABLE_SMARTIESID,
                                SMARTIESTABLE_ID,
                                SMARTIES_TABLE));
        if (query.execPrepared() && (query.numRowsAffected() > 0)) {
            kLogger.warning() << "Removed" << query.numRowsAffected()
                              << "smarties tracks from non-existent smarties";
        }
    }
    {
        // Remove library purged tracks from smarties
        FwdSqlQuery query(database,
                QStringLiteral(
                        "DELETE FROM %1 WHERE %2 NOT IN (SELECT %3 FROM %4)")
                        .arg(SMARTIES_TRACKS_TABLE,
                                SMARTIESTRACKSTABLE_TRACKID,
                                LIBRARYTABLE_ID,
                                LIBRARY_TABLE));
        if (query.execPrepared() && (query.numRowsAffected() > 0)) {
            kLogger.warning() << "Removed" << query.numRowsAffected()
                              << "library purged tracks from smarties";
        }
    }
}

void SmartiesStorage::connectDatabase(const QSqlDatabase& database) {
    m_database = database;
    createViews();
}

void SmartiesStorage::disconnectDatabase() {
    // Ensure that we don't use the current database connection
    // any longer.
    m_database = QSqlDatabase();
}

void SmartiesStorage::createViews() {
    VERIFY_OR_DEBUG_ASSERT(
            FwdSqlQuery(m_database, kSmartiesSummaryViewQuery).execPrepared()) {
        kLogger.critical()
                << "Failed to create database view for smarties summaries!";
    }
}

uint SmartiesStorage::countSmarties() const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT COUNT(*) FROM %1").arg(SMARTIES_TABLE));
    if (query.execPrepared() && query.next()) {
        uint result = query.fieldValue(0).toUInt();
        DEBUG_ASSERT(!query.next());
        return result;
    } else {
        return 0;
    }
}

bool SmartiesStorage::readSmartiesById(SmartiesId id, Smarties* pSmarties) const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT * FROM %1 WHERE %2=:id")
                    .arg(SMARTIES_TABLE, SMARTIESTABLE_ID));
    //                    .arg(SMARTIES_TABLE, SMARTIESTABLE_ID,
    //                    SMARTIESTABLE_SEARCH_SQL));
    //            QStringLiteral("SELECT * FROM %1 WHERE (%2=:id AND %3 IS NOT
    //            NULL AND %4 IS NOT NULL)")
    //                    .arg(SMARTIES_TABLE, SMARTIESTABLE_ID,
    //                    SMARTIESTABLE_SEARCH_INPUT,
    //                    SMARTIESTABLE_SEARCH_SQL));
    query.bindValue(":id", id);
    if (query.execPrepared()) {
        SmartiesSelectResult smarties(std::move(query));
        if ((pSmarties != nullptr) ? smarties.populateNext(pSmarties) : smarties.next()) {
            VERIFY_OR_DEBUG_ASSERT(!smarties.next()) {
                kLogger.warning() << "Ambiguous smarties id: maybe nul values" << id;
            }
            return true;
        } else {
            kLogger.warning() << "Smarties not found by id:" << id;
        }
    }
    return false;
}

bool SmartiesStorage::readSmartiesByName(const QString& name, Smarties* pSmarties) const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT * FROM %1 WHERE %2=:name")
                    .arg(SMARTIES_TABLE, SMARTIESTABLE_NAME));
    query.bindValue(":name", name);
    if (query.execPrepared()) {
        SmartiesSelectResult smarties(std::move(query));
        if ((pSmarties != nullptr) ? smarties.populateNext(pSmarties) : smarties.next()) {
            VERIFY_OR_DEBUG_ASSERT(!smarties.next()) {
                kLogger.warning() << "Ambiguous smarties name:" << name;
            }
            return true;
        } else {
            if (kLogger.debugEnabled()) {
                kLogger.debug() << "Smarties not found by name:" << name;
            }
        }
    }
    return false;
}

SmartiesSelectResult SmartiesStorage::selectSmarties() const {
    FwdSqlQuery query(m_database,
            mixxx::DbConnection::collateLexicographically(
                    QStringLiteral("SELECT * FROM %1 ORDER BY %2")
                            .arg(SMARTIES_TABLE, SMARTIESTABLE_NAME)));

    if (query.execPrepared()) {
        return SmartiesSelectResult(std::move(query));
    } else {
        return SmartiesSelectResult();
    }
}

// SmartiesSelectResult SmartiesStorage::selectSmartiesByIds(
//         const QString& subselectForSmartiesIds,
//         SqlSubselectMode subselectMode) const {
//     QString subselectPrefix;
//     switch (subselectMode) {
//     case SQL_SUBSELECT_IN:
//         if (subselectForSmartiesIds.isEmpty()) {
//             // edge case: no smarties
//             return SmartiesSelectResult();
//         }
//         subselectPrefix = "IN";
//         break;
//     case SQL_SUBSELECT_NOT_IN:
//         if (subselectForSmartiesIds.isEmpty()) {
//             // edge case: all smarties
//             return selectSmarties();
//         }
//         subselectPrefix = "NOT IN";
//         break;
//     }
//     DEBUG_ASSERT(!subselectPrefix.isEmpty());
//     DEBUG_ASSERT(!subselectForSmartiesIds.isEmpty());

//    FwdSqlQuery query(m_database,
//            mixxx::DbConnection::collateLexicographically(
//                    QStringLiteral("SELECT * FROM %1 "
//                                   "WHERE %2 %3 (%4) "
//                                   "ORDER BY %5")
//                            .arg(SMARTIES_TABLE,
//                                    SMARTIESTABLE_ID,
//                                    subselectPrefix,
//                                    subselectForSmartiesIds,
//                                    SMARTIESTABLE_NAME)));

//    if (query.execPrepared()) {
//        return SmartiesSelectResult(std::move(query));
//    } else {
//        return SmartiesSelectResult();
//    }
//}

// SmartiesSelectResult SmartiesStorage::selectAutoDjSmarties(bool autoDjSource) const {
//     FwdSqlQuery query(m_database,
//             mixxx::DbConnection::collateLexicographically(
//                     QStringLiteral("SELECT * FROM %1 WHERE %2=:autoDjSource "
//                                    "ORDER BY %3")
//                             .arg(SMARTIES_TABLE,
//                                     SMARTIESTABLE_AUTODJ_SOURCE,
//                                     SMARTIESTABLE_NAME)));
//     query.bindValue(":autoDjSource", QVariant(autoDjSource));
//     if (query.execPrepared()) {
//         return SmartiesSelectResult(std::move(query));
//     } else {
//         return SmartiesSelectResult();
//     }
// }

SmartiesSummarySelectResult SmartiesStorage::selectSmartiesSummaries() const {
    FwdSqlQuery query(m_database,
            mixxx::DbConnection::collateLexicographically(
                    QStringLiteral("SELECT * FROM %1 ORDER BY %2")
                            .arg(SMARTIES_SUMMARY_VIEW, SMARTIESTABLE_NAME)));
    if (query.execPrepared()) {
        return SmartiesSummarySelectResult(std::move(query));
    } else {
        return SmartiesSummarySelectResult();
    }
}

bool SmartiesStorage::readSmartiesSummaryById(
        SmartiesId id, SmartiesSummary* pSmartiesSummary) const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT * FROM %1 WHERE %2=:id")
                    .arg(SMARTIES_SUMMARY_VIEW, SMARTIESTABLE_ID));
    query.bindValue(":id", id);
    if (query.execPrepared()) {
        SmartiesSummarySelectResult smartiesSummaries(std::move(query));
        if ((pSmartiesSummary != nullptr)
                        ? smartiesSummaries.populateNext(pSmartiesSummary)
                        : smartiesSummaries.next()) {
            VERIFY_OR_DEBUG_ASSERT(!smartiesSummaries.next()) {
                kLogger.warning() << "Ambiguous smarties id:" << id;
            }
            return true;
        } else {
            kLogger.warning() << "Smarties summary not found by id:" << id;
        }
    }
    return false;
}

uint SmartiesStorage::countSmartiesTracks(SmartiesId smartiesId) const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT COUNT(*) FROM %1 WHERE %2=:smartiesId")
                    .arg(SMARTIES_TRACKS_TABLE, SMARTIESTRACKSTABLE_SMARTIESID));
    query.bindValue(":smartiesId", smartiesId);
    if (query.execPrepared() && query.next()) {
        uint result = query.fieldValue(0).toUInt();
        DEBUG_ASSERT(!query.next());
        return result;
    } else {
        return 0;
    }
}

// static
QString SmartiesStorage::formatSubselectQueryForSmartiesTrackIds(SmartiesId smartiesId) {
    return QStringLiteral("SELECT %1 FROM %2 WHERE %3=%4")
            .arg(SMARTIESTRACKSTABLE_TRACKID,
                    SMARTIES_TRACKS_TABLE,
                    SMARTIESTRACKSTABLE_SMARTIESID,
                    smartiesId.toString());
}

QString SmartiesStorage::returnSearchSQLFieldFromTable(SmartiesId smartiesId) {
    return QStringLiteral(" %1.%2=%3")
            .arg(SMARTIES_TABLE,
                    SMARTIESTABLE_ID,
                    smartiesId.toString());
    //    return QStringLiteral("SELECT %1 FROM %2 WHERE %3=%4")
    //            .arg(SMARTIESTABLE_SEARCH_SQL,
    //                    SMARTIES_TABLE,
    //                    SMARTIESTRACKSTABLE_SMARTIESID,
    //                    smartiesId.toString());
}

QString SmartiesStorage::formatQueryForTrackIdsBySmartiesNameLike(
        const QString& smartiesNameLike) const {
    FieldEscaper escaper(m_database);
    QString escapedSmartiesNameLike = escaper.escapeString(
            kSqlLikeMatchAll + smartiesNameLike + kSqlLikeMatchAll);
    return QString(
            "SELECT DISTINCT %1 FROM %2 "
            "JOIN %3 ON %4=%5 WHERE %6 LIKE %7 "
            "ORDER BY %1")
            .arg(SMARTIESTRACKSTABLE_TRACKID,
                    SMARTIES_TRACKS_TABLE,
                    SMARTIES_TABLE,
                    SMARTIESTRACKSTABLE_SMARTIESID,
                    SMARTIESTABLE_ID,
                    SMARTIESTABLE_NAME,
                    escapedSmartiesNameLike);
}

// static
QString SmartiesStorage::formatQueryForTrackIdsWithSmarties() {
    return QStringLiteral(
            "SELECT DISTINCT %1 FROM %2 JOIN %3 ON %4=%5 ORDER BY %1")
            .arg(SMARTIESTRACKSTABLE_TRACKID,
                    SMARTIES_TRACKS_TABLE,
                    SMARTIES_TABLE,
                    SMARTIESTRACKSTABLE_SMARTIESID,
                    SMARTIESTABLE_ID);
}

SmartiesTrackSelectResult SmartiesStorage::selectSmartiesTracksSorted(
        SmartiesId smartiesId) const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT * FROM %1 WHERE %2=:smartiesId ORDER BY %3")
                    .arg(SMARTIES_TRACKS_TABLE,
                            SMARTIESTRACKSTABLE_SMARTIESID,
                            SMARTIESTRACKSTABLE_TRACKID));
    query.bindValue(":smartiesId", smartiesId);
    if (query.execPrepared()) {
        return SmartiesTrackSelectResult(std::move(query));
    } else {
        return SmartiesTrackSelectResult();
    }
}

SmartiesTrackSelectResult SmartiesStorage::selectTrackSmartiesSorted(
        TrackId trackId) const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT * FROM %1 WHERE %2=:trackId ORDER BY %3")
                    .arg(SMARTIES_TRACKS_TABLE,
                            SMARTIESTRACKSTABLE_TRACKID,
                            SMARTIESTRACKSTABLE_SMARTIESID));
    query.bindValue(":trackId", trackId);
    if (query.execPrepared()) {
        return SmartiesTrackSelectResult(std::move(query));
    } else {
        return SmartiesTrackSelectResult();
    }
}

SmartiesSummarySelectResult SmartiesStorage::selectSmartiesWithTrackCount(
        const QList<TrackId>& trackIds) const {
    FwdSqlQuery query(m_database,
            mixxx::DbConnection::collateLexicographically(
                    QStringLiteral("SELECT *, "
                                   "(SELECT COUNT(*) FROM %1 WHERE %2.%3 = %1.%4 and "
                                   "%1.%5 in (%9)) AS %6, "
                                   "0 as %7 FROM %2 ORDER BY %8")
                            .arg(
                                    SMARTIES_TRACKS_TABLE,          // 1
                                    SMARTIES_TABLE,                 // 2
                                    SMARTIESTABLE_ID,               // 3
                                    SMARTIESTRACKSTABLE_SMARTIESID, // 4
                                    SMARTIESTRACKSTABLE_TRACKID,    // 5
                                    SMARTIESSUMMARY_TRACK_COUNT,    // 6
                                    SMARTIESSUMMARY_TRACK_DURATION, // 7
                                    SMARTIESTABLE_NAME,             // 8
                                    joinSqlStringList(trackIds))));

    if (query.execPrepared()) {
        return SmartiesSummarySelectResult(std::move(query));
    } else {
        return SmartiesSummarySelectResult();
    }
}

SmartiesTrackSelectResult SmartiesStorage::selectTracksSortedBySmartiesNameLike(
        const QString& smartiesNameLike) const {
    // TODO: Do SQL LIKE wildcards in smartiesNameLike need to be escaped?
    // Previously we used SqlLikeWildcardEscaper in the past for this
    // purpose. This utility class has become obsolete but could be
    // restored from the 2.3 branch if ever needed again.
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT %1,%2 FROM %3 "
                           "JOIN %4 ON %5 = %6 "
                           "WHERE %7 LIKE :smartiesNameLike "
                           "ORDER BY %1")
                    .arg(SMARTIESTRACKSTABLE_TRACKID,       // 1
                            SMARTIESTRACKSTABLE_SMARTIESID, // 2
                            SMARTIES_TRACKS_TABLE,          // 3
                            SMARTIES_TABLE,                 // 4
                            SMARTIESTABLE_ID,               // 5
                            SMARTIESTRACKSTABLE_SMARTIESID, // 6
                            SMARTIESTABLE_NAME));           // 7
    query.bindValue(":smartiesNameLike",
            QVariant(kSqlLikeMatchAll + smartiesNameLike + kSqlLikeMatchAll));

    if (query.execPrepared()) {
        return SmartiesTrackSelectResult(std::move(query));
    } else {
        return SmartiesTrackSelectResult();
    }
}

// TrackSelectResult SmartiesStorage::selectAllTracksSorted() const {
//     FwdSqlQuery query(m_database,
//             QStringLiteral("SELECT DISTINCT %1 FROM %2 ORDER BY %1")
//                     .arg(SMARTIESTRACKSTABLE_TRACKID, SMARTIES_TRACKS_TABLE));
//     if (query.execPrepared()) {
//         return TrackSelectResult(std::move(query));
//     } else {
//         return TrackSelectResult();
//     }
// }

QSet<SmartiesId> SmartiesStorage::collectSmartiesIdsOfTracks(const QList<TrackId>& trackIds) const {
    // NOTE(uklotzde): One query per track id. This could be optimized
    // by querying for chunks of track ids and collecting the results.
    QSet<SmartiesId> trackSmarties;
    for (const auto& trackId : trackIds) {
        // NOTE(uklotzde): The query result does not need to be sorted by smarties id
        // here. But since the corresponding FK column is indexed the impact on the
        // performance should be negligible. By reusing an existing query we reduce
        // the amount of code and the number of prepared SQL queries.
        SmartiesTrackSelectResult smartiesTracks(selectTrackSmartiesSorted(trackId));
        while (smartiesTracks.next()) {
            DEBUG_ASSERT(smartiesTracks.trackId() == trackId);
            trackSmarties.insert(smartiesTracks.smartiesId());
        }
    }
    return trackSmarties;
}

bool SmartiesStorage::onInsertingSmarties(
        const Smarties& smarties,
        SmartiesId* pSmartiesId) {
    VERIFY_OR_DEBUG_ASSERT(!smarties.getId().isValid()) {
        kLogger.warning()
                << "Cannot insert smarties with a valid id:" << smarties.getId();
        return false;
    }
    FwdSqlQuery query(m_database,
            QStringLiteral(
                    "INSERT INTO %1 (%2,%3,%4,%5,%6) "
                    "VALUES (:name,:locked,:autoDjSource, :searchInput, :searchSql)")
                    .arg(
                            SMARTIES_TABLE,
                            SMARTIESTABLE_NAME,
                            SMARTIESTABLE_LOCKED,
                            SMARTIESTABLE_AUTODJ_SOURCE,
                            SMARTIESTABLE_SEARCH_INPUT,
                            SMARTIESTABLE_SEARCH_SQL));
    VERIFY_OR_DEBUG_ASSERT(query.isPrepared()) {
        return false;
    }
    SmartiesQueryBinder queryBinder(query);
    queryBinder.bindName(":name", smarties);
    queryBinder.bindLocked(":locked", smarties);
    queryBinder.bindAutoDjSource(":autoDjSource", smarties);
    queryBinder.bindSearchInput(":searchInput", smarties);
    queryBinder.bindSearchSql(":searchSql", smarties);
    VERIFY_OR_DEBUG_ASSERT(query.execPrepared()) {
        return false;
    }
    if (query.numRowsAffected() > 0) {
        DEBUG_ASSERT(query.numRowsAffected() == 1);
        if (pSmartiesId != nullptr) {
            *pSmartiesId = SmartiesId(query.lastInsertId());
            DEBUG_ASSERT(pSmartiesId->isValid());
        }
        return true;
    } else {
        return false;
    }
}

bool SmartiesStorage::onUpdatingSmarties(
        const Smarties& smarties) {
    VERIFY_OR_DEBUG_ASSERT(smarties.getId().isValid()) {
        kLogger.warning()
                << "Cannot update smarties without a valid id";
        return false;
    }
    FwdSqlQuery query(m_database,
            QString(
                    "UPDATE %1 "
                    "SET %2=:name,%3=:locked,%4=:autoDjSource, %5=:searchInput, %6=:searchSql "
                    "WHERE %7=:id")
                    .arg(
                            SMARTIES_TABLE,
                            SMARTIESTABLE_NAME,
                            SMARTIESTABLE_LOCKED,
                            SMARTIESTABLE_AUTODJ_SOURCE,
                            SMARTIESTABLE_SEARCH_INPUT,
                            SMARTIESTABLE_SEARCH_SQL,
                            SMARTIESTABLE_ID));
    VERIFY_OR_DEBUG_ASSERT(query.isPrepared()) {
        return false;
    }
    SmartiesQueryBinder queryBinder(query);
    queryBinder.bindId(":id", smarties);
    queryBinder.bindName(":name", smarties);
    queryBinder.bindLocked(":locked", smarties);
    queryBinder.bindAutoDjSource(":autoDjSource", smarties);
    queryBinder.bindSearchInput(":searchInput", smarties);
    queryBinder.bindSearchSql(":searchSql", smarties);
    VERIFY_OR_DEBUG_ASSERT(query.execPrepared()) {
        return false;
    }
    if (query.numRowsAffected() > 0) {
        VERIFY_OR_DEBUG_ASSERT(query.numRowsAffected() <= 1) {
            kLogger.warning()
                    << "Updated multiple smarties with the same id" << smarties.getId();
        }
        return true;
    } else {
        kLogger.warning()
                << "Cannot update non-existent smarties with id" << smarties.getId();
        return false;
    }
}

bool SmartiesStorage::onDeletingSmarties(
        SmartiesId smartiesId) {
    VERIFY_OR_DEBUG_ASSERT(smartiesId.isValid()) {
        kLogger.warning()
                << "Cannot delete smarties without a valid id";
        return false;
    }
    {
        FwdSqlQuery query(m_database,
                QStringLiteral("DELETE FROM %1 WHERE %2=:id")
                        .arg(SMARTIES_TRACKS_TABLE, SMARTIESTRACKSTABLE_SMARTIESID));
        VERIFY_OR_DEBUG_ASSERT(query.isPrepared()) {
            return false;
        }
        query.bindValue(":id", smartiesId);
        VERIFY_OR_DEBUG_ASSERT(query.execPrepared()) {
            return false;
        }
        if (query.numRowsAffected() <= 0) {
            if (kLogger.debugEnabled()) {
                kLogger.debug()
                        << "Deleting empty smarties with id"
                        << smartiesId;
            }
        }
    }
    {
        FwdSqlQuery query(m_database,
                QStringLiteral("DELETE FROM %1 WHERE %2=:id")
                        .arg(SMARTIES_TABLE, SMARTIESTABLE_ID));
        VERIFY_OR_DEBUG_ASSERT(query.isPrepared()) {
            return false;
        }
        query.bindValue(":id", smartiesId);
        VERIFY_OR_DEBUG_ASSERT(query.execPrepared()) {
            return false;
        }
        if (query.numRowsAffected() > 0) {
            VERIFY_OR_DEBUG_ASSERT(query.numRowsAffected() <= 1) {
                kLogger.warning()
                        << "Deleted multiple smarties with the same id" << smartiesId;
            }
            return true;
        } else {
            kLogger.warning()
                    << "Cannot delete non-existent smarties with id" << smartiesId;
            return false;
        }
    }
}

bool SmartiesStorage::onAddingSmartiesTracks(
        SmartiesId smartiesId,
        const QList<TrackId>& trackIds) {
    FwdSqlQuery query(m_database,
            QStringLiteral(
                    "INSERT OR IGNORE INTO %1 (%2, %3) "
                    "VALUES (:smartiesId,:trackId)")
                    .arg(
                            SMARTIES_TRACKS_TABLE,
                            SMARTIESTRACKSTABLE_SMARTIESID,
                            SMARTIESTRACKSTABLE_TRACKID));
    if (!query.isPrepared()) {
        return false;
    }
    query.bindValue(":smartiesId", smartiesId);
    for (const auto& trackId : trackIds) {
        query.bindValue(":trackId", trackId);
        if (!query.execPrepared()) {
            return false;
        }
        if (query.numRowsAffected() == 0) {
            // track is already in smarties
            if (kLogger.debugEnabled()) {
                kLogger.debug()
                        << "Track" << trackId
                        << "not added to smarties" << smartiesId;
            }
        } else {
            DEBUG_ASSERT(query.numRowsAffected() == 1);
        }
    }
    return true;
}

bool SmartiesStorage::onRemovingSmartiesTracks(
        SmartiesId smartiesId,
        const QList<TrackId>& trackIds) {
    // NOTE(uklotzde): We remove tracks in a loop
    // analogously to adding tracks (see above).
    FwdSqlQuery query(m_database,
            QStringLiteral(
                    "DELETE FROM %1 "
                    "WHERE %2=:smartiesId AND %3=:trackId")
                    .arg(
                            SMARTIES_TRACKS_TABLE,
                            SMARTIESTRACKSTABLE_SMARTIESID,
                            SMARTIESTRACKSTABLE_TRACKID));
    if (!query.isPrepared()) {
        return false;
    }
    query.bindValue(":smartiesId", smartiesId);
    for (const auto& trackId : trackIds) {
        query.bindValue(":trackId", trackId);
        if (!query.execPrepared()) {
            return false;
        }
        if (query.numRowsAffected() == 0) {
            // track not found in smarties
            if (kLogger.debugEnabled()) {
                kLogger.debug()
                        << "Track" << trackId
                        << "not removed from smarties" << smartiesId;
            }
        } else {
            DEBUG_ASSERT(query.numRowsAffected() == 1);
        }
    }
    return true;
}

bool SmartiesStorage::onPurgingTracks(
        const QList<TrackId>& trackIds) {
    // NOTE(uklotzde): Remove tracks from smarties one-by-one.
    // This might be optimized by deleting multiple track ids
    // at once in chunks with a maximum size.
    FwdSqlQuery query(m_database,
            QStringLiteral("DELETE FROM %1 WHERE %2=:trackId")
                    .arg(SMARTIES_TRACKS_TABLE, SMARTIESTRACKSTABLE_TRACKID));
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
