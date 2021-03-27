#include "library/trackset/crate/cratestorage.h"

#include "library/dao/trackschema.h"
#include "library/queryutil.h"
#include "library/trackset/crate/crateschema.h"
#include "util/db/dbconnection.h"
#include "util/db/fwdsqlquery.h"
#include "util/db/sqllikewildcards.h"
#include "util/logger.h"

namespace {

const mixxx::Logger kLogger("CrateStorage");

const QString CRATETABLE_LOCKED = "locked";

const QString CRATE_SUMMARY_VIEW = "crate_summary";

const QString CRATESUMMARY_TRACK_COUNT = "track_count";
const QString CRATESUMMARY_TRACK_DURATION = "track_duration";

const QString kCrateTracksJoin =
        QStringLiteral("LEFT JOIN %3 ON %3.%4=%1.%2")
                .arg(CRATE_TABLE, CRATETABLE_ID, CRATE_TRACKS_TABLE, CRATETRACKSTABLE_CRATEID);

const QString kLibraryTracksJoin = kCrateTracksJoin +
        QStringLiteral(" LEFT JOIN %3 ON %3.%4=%1.%2")
                .arg(CRATE_TRACKS_TABLE, CRATETRACKSTABLE_TRACKID, LIBRARY_TABLE, LIBRARYTABLE_ID);

const QString kCrateSummaryViewSelect =
        QStringLiteral(
                "SELECT %1.*,"
                "COUNT(CASE %2.%4 WHEN 0 THEN 1 ELSE NULL END) AS %5,"
                "SUM(CASE %2.%4 WHEN 0 THEN %2.%3 ELSE 0 END) AS %6 "
                "FROM %1")
                .arg(
                        CRATE_TABLE,
                        LIBRARY_TABLE,
                        LIBRARYTABLE_DURATION,
                        LIBRARYTABLE_MIXXXDELETED,
                        CRATESUMMARY_TRACK_COUNT,
                        CRATESUMMARY_TRACK_DURATION);

const QString kCrateSummaryViewQuery =
        QStringLiteral(
                "CREATE TEMPORARY VIEW IF NOT EXISTS %1 AS %2 %3 "
                "GROUP BY %4.%5")
                .arg(
                        CRATE_SUMMARY_VIEW,
                        kCrateSummaryViewSelect,
                        kLibraryTracksJoin,
                        CRATE_TABLE,
                        CRATETABLE_ID);

class CrateQueryBinder {
  public:
    explicit CrateQueryBinder(FwdSqlQuery& query)
            : m_query(query) {
    }
    virtual ~CrateQueryBinder() = default;

    void bindId(const QString& placeholder, const Crate& crate) const {
        m_query.bindValue(placeholder, crate.getId());
    }
    void bindName(const QString& placeholder, const Crate& crate) const {
        m_query.bindValue(placeholder, crate.getName());
    }
    void bindLocked(const QString& placeholder, const Crate& crate) const {
        m_query.bindValue(placeholder, crate.isLocked());
    }
    void bindAutoDjSource(const QString& placeholder, const Crate& crate) const {
        m_query.bindValue(placeholder, crate.isAutoDjSource());
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

CrateQueryFields::CrateQueryFields(const FwdSqlQuery& query)
        : m_iId(query.fieldIndex(CRATETABLE_ID)),
          m_iName(query.fieldIndex(CRATETABLE_NAME)),
          m_iLocked(query.fieldIndex(CRATETABLE_LOCKED)),
          m_iAutoDjSource(query.fieldIndex(CRATETABLE_AUTODJ_SOURCE)) {
}

void CrateQueryFields::populateFromQuery(
        const FwdSqlQuery& query,
        Crate* pCrate) const {
    pCrate->setId(getId(query));
    pCrate->setName(getName(query));
    pCrate->setLocked(isLocked(query));
    pCrate->setAutoDjSource(isAutoDjSource(query));
}

CrateTrackQueryFields::CrateTrackQueryFields(const FwdSqlQuery& query)
        : m_iCrateId(query.fieldIndex(CRATETRACKSTABLE_CRATEID)),
          m_iTrackId(query.fieldIndex(CRATETRACKSTABLE_TRACKID)) {
}

TrackQueryFields::TrackQueryFields(const FwdSqlQuery& query)
        : m_iTrackId(query.fieldIndex(CRATETRACKSTABLE_TRACKID)) {
}

CrateSummaryQueryFields::CrateSummaryQueryFields(const FwdSqlQuery& query)
        : CrateQueryFields(query),
          m_iTrackCount(query.fieldIndex(CRATESUMMARY_TRACK_COUNT)),
          m_iTrackDuration(query.fieldIndex(CRATESUMMARY_TRACK_DURATION)) {
}

void CrateSummaryQueryFields::populateFromQuery(
        const FwdSqlQuery& query,
        CrateSummary* pCrateSummary) const {
    CrateQueryFields::populateFromQuery(query, pCrateSummary);
    pCrateSummary->setTrackCount(getTrackCount(query));
    pCrateSummary->setTrackDuration(getTrackDuration(query));
}

void CrateStorage::repairDatabase(const QSqlDatabase& database) {
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

    // Crates
    {
        // Delete crates with empty names
        FwdSqlQuery query(database,
                QStringLiteral("DELETE FROM %1 WHERE %2 IS NULL OR TRIM(%2)=''")
                        .arg(CRATE_TABLE, CRATETABLE_NAME));
        if (query.execPrepared() && (query.numRowsAffected() > 0)) {
            kLogger.warning()
                    << "Deleted" << query.numRowsAffected()
                    << "crates with empty names";
        }
    }
    {
        // Fix invalid values in the "locked" column
        FwdSqlQuery query(database,
                QStringLiteral("UPDATE %1 SET %2=0 WHERE %2 NOT IN (0,1)")
                        .arg(CRATE_TABLE, CRATETABLE_LOCKED));
        if (query.execPrepared() && (query.numRowsAffected() > 0)) {
            kLogger.warning()
                    << "Fixed boolean values in table" << CRATE_TABLE
                    << "column" << CRATETABLE_LOCKED
                    << "for" << query.numRowsAffected() << "crates";
        }
    }
    {
        // Fix invalid values in the "autodj_source" column
        FwdSqlQuery query(database,
                QStringLiteral("UPDATE %1 SET %2=0 WHERE %2 NOT IN (0,1)")
                        .arg(CRATE_TABLE, CRATETABLE_AUTODJ_SOURCE));
        if (query.execPrepared() && (query.numRowsAffected() > 0)) {
            kLogger.warning()
                    << "Fixed boolean values in table" << CRATE_TABLE
                    << "column" << CRATETABLE_AUTODJ_SOURCE
                    << "for" << query.numRowsAffected() << "crates";
        }
    }

    // Crate tracks
    {
        // Remove tracks from non-existent crates
        FwdSqlQuery query(database,
                QStringLiteral(
                        "DELETE FROM %1 WHERE %2 NOT IN (SELECT %3 FROM %4)")
                        .arg(CRATE_TRACKS_TABLE,
                                CRATETRACKSTABLE_CRATEID,
                                CRATETABLE_ID,
                                CRATE_TABLE));
        if (query.execPrepared() && (query.numRowsAffected() > 0)) {
            kLogger.warning() << "Removed" << query.numRowsAffected()
                              << "crate tracks from non-existent crates";
        }
    }
    {
        // Remove library purged tracks from crates
        FwdSqlQuery query(database,
                QStringLiteral(
                        "DELETE FROM %1 WHERE %2 NOT IN (SELECT %3 FROM %4)")
                        .arg(CRATE_TRACKS_TABLE,
                                CRATETRACKSTABLE_TRACKID,
                                LIBRARYTABLE_ID,
                                LIBRARY_TABLE));
        if (query.execPrepared() && (query.numRowsAffected() > 0)) {
            kLogger.warning() << "Removed" << query.numRowsAffected()
                              << "library purged tracks from crates";
        }
    }
}

void CrateStorage::connectDatabase(const QSqlDatabase& database) {
    m_database = database;
    createViews();
}

void CrateStorage::disconnectDatabase() {
    // Ensure that we don't use the current database connection
    // any longer.
    m_database = QSqlDatabase();
}

void CrateStorage::createViews() {
    VERIFY_OR_DEBUG_ASSERT(
            FwdSqlQuery(m_database, kCrateSummaryViewQuery).execPrepared()) {
        kLogger.critical()
                << "Failed to create database view for crate summaries!";
    }
}

uint CrateStorage::countCrates() const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT COUNT(*) FROM %1").arg(CRATE_TABLE));
    if (query.execPrepared() && query.next()) {
        uint result = query.fieldValue(0).toUInt();
        DEBUG_ASSERT(!query.next());
        return result;
    } else {
        return 0;
    }
}

bool CrateStorage::readCrateById(CrateId id, Crate* pCrate) const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT * FROM %1 WHERE %2=:id")
                    .arg(CRATE_TABLE, CRATETABLE_ID));
    query.bindValue(":id", id);
    if (query.execPrepared()) {
        CrateSelectResult crates(std::move(query));
        if ((pCrate != nullptr) ? crates.populateNext(pCrate) : crates.next()) {
            VERIFY_OR_DEBUG_ASSERT(!crates.next()) {
                kLogger.warning() << "Ambiguous crate id:" << id;
            }
            return true;
        } else {
            kLogger.warning() << "Crate not found by id:" << id;
        }
    }
    return false;
}

bool CrateStorage::readCrateByName(const QString& name, Crate* pCrate) const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT * FROM %1 WHERE %2=:name")
                    .arg(CRATE_TABLE, CRATETABLE_NAME));
    query.bindValue(":name", name);
    if (query.execPrepared()) {
        CrateSelectResult crates(std::move(query));
        if ((pCrate != nullptr) ? crates.populateNext(pCrate) : crates.next()) {
            VERIFY_OR_DEBUG_ASSERT(!crates.next()) {
                kLogger.warning() << "Ambiguous crate name:" << name;
            }
            return true;
        } else {
            if (kLogger.debugEnabled()) {
                kLogger.debug() << "Crate not found by name:" << name;
            }
        }
    }
    return false;
}

CrateSelectResult CrateStorage::selectCrates() const {
    FwdSqlQuery query(m_database,
            mixxx::DbConnection::collateLexicographically(
                    QStringLiteral("SELECT * FROM %1 ORDER BY %2")
                            .arg(CRATE_TABLE, CRATETABLE_NAME)));

    if (query.execPrepared()) {
        return CrateSelectResult(std::move(query));
    } else {
        return CrateSelectResult();
    }
}

CrateSelectResult CrateStorage::selectCratesByIds(
        const QString& subselectForCrateIds,
        SqlSubselectMode subselectMode) const {
    QString subselectPrefix;
    switch (subselectMode) {
    case SQL_SUBSELECT_IN:
        if (subselectForCrateIds.isEmpty()) {
            // edge case: no crates
            return CrateSelectResult();
        }
        subselectPrefix = "IN";
        break;
    case SQL_SUBSELECT_NOT_IN:
        if (subselectForCrateIds.isEmpty()) {
            // edge case: all crates
            return selectCrates();
        }
        subselectPrefix = "NOT IN";
        break;
    }
    DEBUG_ASSERT(!subselectPrefix.isEmpty());
    DEBUG_ASSERT(!subselectForCrateIds.isEmpty());

    FwdSqlQuery query(m_database,
            mixxx::DbConnection::collateLexicographically(
                    QStringLiteral("SELECT * FROM %1 "
                                   "WHERE %2 %3 (%4) "
                                   "ORDER BY %5")
                            .arg(CRATE_TABLE,
                                    CRATETABLE_ID,
                                    subselectPrefix,
                                    subselectForCrateIds,
                                    CRATETABLE_NAME)));

    if (query.execPrepared()) {
        return CrateSelectResult(std::move(query));
    } else {
        return CrateSelectResult();
    }
}

CrateSelectResult CrateStorage::selectAutoDjCrates(bool autoDjSource) const {
    FwdSqlQuery query(m_database,
            mixxx::DbConnection::collateLexicographically(
                    QStringLiteral("SELECT * FROM %1 WHERE %2=:autoDjSource "
                                   "ORDER BY %3")
                            .arg(CRATE_TABLE,
                                    CRATETABLE_AUTODJ_SOURCE,
                                    CRATETABLE_NAME)));
    query.bindValue(":autoDjSource", autoDjSource);
    if (query.execPrepared()) {
        return CrateSelectResult(std::move(query));
    } else {
        return CrateSelectResult();
    }
}

CrateSummarySelectResult CrateStorage::selectCrateSummaries() const {
    FwdSqlQuery query(m_database,
            mixxx::DbConnection::collateLexicographically(
                    QStringLiteral("SELECT * FROM %1 ORDER BY %2")
                            .arg(CRATE_SUMMARY_VIEW, CRATETABLE_NAME)));
    if (query.execPrepared()) {
        return CrateSummarySelectResult(std::move(query));
    } else {
        return CrateSummarySelectResult();
    }
}

bool CrateStorage::readCrateSummaryById(
        CrateId id, CrateSummary* pCrateSummary) const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT * FROM %1 WHERE %2=:id")
                    .arg(CRATE_SUMMARY_VIEW, CRATETABLE_ID));
    query.bindValue(":id", id);
    if (query.execPrepared()) {
        CrateSummarySelectResult crateSummaries(std::move(query));
        if ((pCrateSummary != nullptr)
                        ? crateSummaries.populateNext(pCrateSummary)
                        : crateSummaries.next()) {
            VERIFY_OR_DEBUG_ASSERT(!crateSummaries.next()) {
                kLogger.warning() << "Ambiguous crate id:" << id;
            }
            return true;
        } else {
            kLogger.warning() << "Crate summary not found by id:" << id;
        }
    }
    return false;
}

uint CrateStorage::countCrateTracks(CrateId crateId) const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT COUNT(*) FROM %1 WHERE %2=:crateId")
                    .arg(CRATE_TRACKS_TABLE, CRATETRACKSTABLE_CRATEID));
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
QString CrateStorage::formatSubselectQueryForCrateTrackIds(CrateId crateId) {
    return QStringLiteral("SELECT %1 FROM %2 WHERE %3=%4")
            .arg(CRATETRACKSTABLE_TRACKID,
                    CRATE_TRACKS_TABLE,
                    CRATETRACKSTABLE_CRATEID,
                    crateId.toString());
}

QString CrateStorage::formatQueryForTrackIdsByCrateNameLike(
        const QString& crateNameLike) const {
    FieldEscaper escaper(m_database);
    QString escapedCrateNameLike = escaper.escapeString(
            kSqlLikeMatchAll + crateNameLike + kSqlLikeMatchAll);
    return QString(
            "SELECT DISTINCT %1 FROM %2 "
            "JOIN %3 ON %4=%5 WHERE %6 LIKE %7 "
            "ORDER BY %1")
            .arg(CRATETRACKSTABLE_TRACKID,
                    CRATE_TRACKS_TABLE,
                    CRATE_TABLE,
                    CRATETRACKSTABLE_CRATEID,
                    CRATETABLE_ID,
                    CRATETABLE_NAME,
                    escapedCrateNameLike);
}

//static
QString CrateStorage::formatQueryForTrackIdsWithCrate() {
    return QStringLiteral(
            "SELECT DISTINCT %1 FROM %2 JOIN %3 ON %4=%5 ORDER BY %1")
            .arg(CRATETRACKSTABLE_TRACKID,
                    CRATE_TRACKS_TABLE,
                    CRATE_TABLE,
                    CRATETRACKSTABLE_CRATEID,
                    CRATETABLE_ID);
}

CrateTrackSelectResult CrateStorage::selectCrateTracksSorted(
        CrateId crateId) const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT * FROM %1 WHERE %2=:crateId ORDER BY %3")
                    .arg(CRATE_TRACKS_TABLE,
                            CRATETRACKSTABLE_CRATEID,
                            CRATETRACKSTABLE_TRACKID));
    query.bindValue(":crateId", crateId);
    if (query.execPrepared()) {
        return CrateTrackSelectResult(std::move(query));
    } else {
        return CrateTrackSelectResult();
    }
}

CrateTrackSelectResult CrateStorage::selectTrackCratesSorted(
        TrackId trackId) const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT * FROM %1 WHERE %2=:trackId ORDER BY %3")
                    .arg(CRATE_TRACKS_TABLE,
                            CRATETRACKSTABLE_TRACKID,
                            CRATETRACKSTABLE_CRATEID));
    query.bindValue(":trackId", trackId);
    if (query.execPrepared()) {
        return CrateTrackSelectResult(std::move(query));
    } else {
        return CrateTrackSelectResult();
    }
}

CrateSummarySelectResult CrateStorage::selectCratesWithTrackCount(
        const QList<TrackId>& trackIds) const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT *, "
                           "(SELECT COUNT(*) FROM %1 WHERE %2.%3 = %1.%4 and "
                           "%1.%5 in (%9)) AS %6, "
                           "0 as %7 FROM %2 ORDER BY %8")
                    .arg(
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

CrateTrackSelectResult CrateStorage::selectTracksSortedByCrateNameLike(
        const QString& crateNameLike) const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT %1,%2 FROM %3 "
                           "JOIN %4 ON %5 = %6 "
                           "WHERE %7 LIKE :crateNameLike "
                           "ORDER BY %1")
                    .arg(CRATETRACKSTABLE_TRACKID,
                            CRATETRACKSTABLE_CRATEID,
                            CRATE_TRACKS_TABLE,
                            CRATE_TABLE,
                            CRATETABLE_ID,
                            CRATETRACKSTABLE_CRATEID,
                            CRATETABLE_NAME));
    query.bindValue(":crateNameLike",
            QVariant(kSqlLikeMatchAll + crateNameLike + kSqlLikeMatchAll));

    if (query.execPrepared()) {
        return CrateTrackSelectResult(std::move(query));
    } else {
        return CrateTrackSelectResult();
    }
}

TrackSelectResult CrateStorage::selectAllTracksSorted() const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT DISTINCT %1 FROM %2 ORDER BY %1")
                    .arg(CRATETRACKSTABLE_TRACKID, CRATE_TRACKS_TABLE));
    if (query.execPrepared()) {
        return TrackSelectResult(std::move(query));
    } else {
        return TrackSelectResult();
    }
}

QSet<CrateId> CrateStorage::collectCrateIdsOfTracks(const QList<TrackId>& trackIds) const {
    // NOTE(uklotzde): One query per track id. This could be optimized
    // by querying for chunks of track ids and collecting the results.
    QSet<CrateId> trackCrates;
    for (const auto& trackId : trackIds) {
        // NOTE(uklotzde): The query result does not need to be sorted by crate id
        // here. But since the corresponding FK column is indexed the impact on the
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

bool CrateStorage::onInsertingCrate(
        const Crate& crate,
        CrateId* pCrateId) {
    VERIFY_OR_DEBUG_ASSERT(!crate.getId().isValid()) {
        kLogger.warning()
                << "Cannot insert crate with a valid id:" << crate.getId();
        return false;
    }
    FwdSqlQuery query(m_database,
            QStringLiteral(
                    "INSERT INTO %1 (%2,%3,%4) "
                    "VALUES (:name,:locked,:autoDjSource)")
                    .arg(
                            CRATE_TABLE,
                            CRATETABLE_NAME,
                            CRATETABLE_LOCKED,
                            CRATETABLE_AUTODJ_SOURCE));
    VERIFY_OR_DEBUG_ASSERT(query.isPrepared()) {
        return false;
    }
    CrateQueryBinder queryBinder(query);
    queryBinder.bindName(":name", crate);
    queryBinder.bindLocked(":locked", crate);
    queryBinder.bindAutoDjSource(":autoDjSource", crate);
    VERIFY_OR_DEBUG_ASSERT(query.execPrepared()) {
        return false;
    }
    if (query.numRowsAffected() > 0) {
        DEBUG_ASSERT(query.numRowsAffected() == 1);
        if (pCrateId != nullptr) {
            *pCrateId = CrateId(query.lastInsertId());
            DEBUG_ASSERT(pCrateId->isValid());
        }
        return true;
    } else {
        return false;
    }
}

bool CrateStorage::onUpdatingCrate(
        const Crate& crate) {
    VERIFY_OR_DEBUG_ASSERT(crate.getId().isValid()) {
        kLogger.warning()
                << "Cannot update crate without a valid id";
        return false;
    }
    FwdSqlQuery query(m_database,
            QString(
                    "UPDATE %1 "
                    "SET %2=:name,%3=:locked,%4=:autoDjSource "
                    "WHERE %5=:id")
                    .arg(
                            CRATE_TABLE,
                            CRATETABLE_NAME,
                            CRATETABLE_LOCKED,
                            CRATETABLE_AUTODJ_SOURCE,
                            CRATETABLE_ID));
    VERIFY_OR_DEBUG_ASSERT(query.isPrepared()) {
        return false;
    }
    CrateQueryBinder queryBinder(query);
    queryBinder.bindId(":id", crate);
    queryBinder.bindName(":name", crate);
    queryBinder.bindLocked(":locked", crate);
    queryBinder.bindAutoDjSource(":autoDjSource", crate);
    VERIFY_OR_DEBUG_ASSERT(query.execPrepared()) {
        return false;
    }
    if (query.numRowsAffected() > 0) {
        VERIFY_OR_DEBUG_ASSERT(query.numRowsAffected() <= 1) {
            kLogger.warning()
                    << "Updated multiple crates with the same id" << crate.getId();
        }
        return true;
    } else {
        kLogger.warning()
                << "Cannot update non-existent crate with id" << crate.getId();
        return false;
    }
}

bool CrateStorage::onDeletingCrate(
        CrateId crateId) {
    VERIFY_OR_DEBUG_ASSERT(crateId.isValid()) {
        kLogger.warning()
                << "Cannot delete crate without a valid id";
        return false;
    }
    {
        FwdSqlQuery query(m_database,
                QStringLiteral("DELETE FROM %1 WHERE %2=:id")
                        .arg(CRATE_TRACKS_TABLE, CRATETRACKSTABLE_CRATEID));
        VERIFY_OR_DEBUG_ASSERT(query.isPrepared()) {
            return false;
        }
        query.bindValue(":id", crateId);
        VERIFY_OR_DEBUG_ASSERT(query.execPrepared()) {
            return false;
        }
        if (query.numRowsAffected() <= 0) {
            if (kLogger.debugEnabled()) {
                kLogger.debug()
                        << "Deleting empty crate with id"
                        << crateId;
            }
        }
    }
    {
        FwdSqlQuery query(m_database,
                QStringLiteral("DELETE FROM %1 WHERE %2=:id")
                        .arg(CRATE_TABLE, CRATETABLE_ID));
        VERIFY_OR_DEBUG_ASSERT(query.isPrepared()) {
            return false;
        }
        query.bindValue(":id", crateId);
        VERIFY_OR_DEBUG_ASSERT(query.execPrepared()) {
            return false;
        }
        if (query.numRowsAffected() > 0) {
            VERIFY_OR_DEBUG_ASSERT(query.numRowsAffected() <= 1) {
                kLogger.warning()
                        << "Deleted multiple crates with the same id" << crateId;
            }
            return true;
        } else {
            kLogger.warning()
                    << "Cannot delete non-existent crate with id" << crateId;
            return false;
        }
    }
}

bool CrateStorage::onAddingCrateTracks(
        CrateId crateId,
        const QList<TrackId>& trackIds) {
    FwdSqlQuery query(m_database,
            QStringLiteral(
                    "INSERT OR IGNORE INTO %1 (%2, %3) "
                    "VALUES (:crateId,:trackId)")
                    .arg(
                            CRATE_TRACKS_TABLE,
                            CRATETRACKSTABLE_CRATEID,
                            CRATETRACKSTABLE_TRACKID));
    if (!query.isPrepared()) {
        return false;
    }
    query.bindValue(":crateId", crateId);
    for (const auto& trackId : trackIds) {
        query.bindValue(":trackId", trackId);
        if (!query.execPrepared()) {
            return false;
        }
        if (query.numRowsAffected() == 0) {
            // track is already in crate
            if (kLogger.debugEnabled()) {
                kLogger.debug()
                        << "Track" << trackId
                        << "not added to crate" << crateId;
            }
        } else {
            DEBUG_ASSERT(query.numRowsAffected() == 1);
        }
    }
    return true;
}

bool CrateStorage::onRemovingCrateTracks(
        CrateId crateId,
        const QList<TrackId>& trackIds) {
    // NOTE(uklotzde): We remove tracks in a loop
    // analogously to adding tracks (see above).
    FwdSqlQuery query(m_database,
            QStringLiteral(
                    "DELETE FROM %1 "
                    "WHERE %2=:crateId AND %3=:trackId")
                    .arg(
                            CRATE_TRACKS_TABLE,
                            CRATETRACKSTABLE_CRATEID,
                            CRATETRACKSTABLE_TRACKID));
    if (!query.isPrepared()) {
        return false;
    }
    query.bindValue(":crateId", crateId);
    for (const auto& trackId : trackIds) {
        query.bindValue(":trackId", trackId);
        if (!query.execPrepared()) {
            return false;
        }
        if (query.numRowsAffected() == 0) {
            // track not found in crate
            if (kLogger.debugEnabled()) {
                kLogger.debug()
                        << "Track" << trackId
                        << "not removed from crate" << crateId;
            }
        } else {
            DEBUG_ASSERT(query.numRowsAffected() == 1);
        }
    }
    return true;
}

bool CrateStorage::onPurgingTracks(
        const QList<TrackId>& trackIds) {
    // NOTE(uklotzde): Remove tracks from crates one-by-one.
    // This might be optimized by deleting multiple track ids
    // at once in chunks with a maximum size.
    FwdSqlQuery query(m_database,
            QStringLiteral("DELETE FROM %1 WHERE %2=:trackId")
                    .arg(CRATE_TRACKS_TABLE, CRATETRACKSTABLE_TRACKID));
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
