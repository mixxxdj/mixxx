#include <QVector>

#include "library/features/crates/cratestorage.h"

#include "library/crate/crateschema.h"
#include "library/dao/trackschema.h"
#include "library/queryutil.h"

#include "util/db/dbconnection.h"

#include "util/db/sqllikewildcards.h"
#include "util/db/fwdsqlquery.h"

#include "util/logger.h"


namespace {

const mixxx::Logger kLogger("CrateStorage");

} // anonymus namespace

void CrateStorage::repairDatabase(QSqlDatabase database) {
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
        FwdSqlQuery query(database, QString(
                "DELETE FROM %1 WHERE %2 IS NULL OR TRIM(%2)=''").arg(
                        CRATE_TABLE,
                        CRATETABLE_NAME));
        if (query.execPrepared() && (query.numRowsAffected() > 0)) {
            kLogger.warning()
                    << "Deleted" << query.numRowsAffected()
                    << "crates with empty names";
        }
    }
    {
        // Fix invalid values in the "locked" column
        FwdSqlQuery query(database, QString(
                "UPDATE %1 SET %2=0 WHERE %2 NOT IN (0,1)").arg(
                        CRATE_TABLE,
                        CRATETABLE_LOCKED));
        if (query.execPrepared() && (query.numRowsAffected() > 0)) {
            kLogger.warning()
                    << "Fixed boolean values in table" << CRATE_TABLE
                    << "column" << CRATETABLE_LOCKED
                    << "for" << query.numRowsAffected() << "crates";
        }
    }
    {
        // Fix invalid values in the "autodj_source" column
        FwdSqlQuery query(database, QString(
                "UPDATE %1 SET %2=0 WHERE %2 NOT IN (0,1)").arg(
                        CRATE_TABLE,
                        CRATETABLE_AUTODJ_SOURCE));
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
        FwdSqlQuery query(database, QString(
                "DELETE FROM %1 WHERE %2 NOT IN (SELECT %3 FROM %4)").arg(
                        CRATE_TRACKS_TABLE,
                        CRATETRACKSTABLE_CRATEID,
                        CRATETABLE_ID,
                        CRATE_TABLE));
        if (query.execPrepared() && (query.numRowsAffected() > 0)) {
            kLogger.warning()
                    << "Removed" << query.numRowsAffected()
                    << "crate tracks from non-existent crates";
        }
    }
    {
        // Remove library purged tracks from crates
        FwdSqlQuery query(database, QString(
                "DELETE FROM %1 WHERE %2 NOT IN (SELECT %3 FROM %4)").arg(
                        CRATE_TRACKS_TABLE,
                        CRATETRACKSTABLE_TRACKID,
                        LIBRARYTABLE_ID,
                        LIBRARY_TABLE));
        if (query.execPrepared() && (query.numRowsAffected() > 0)) {
            kLogger.warning()
                    << "Removed" << query.numRowsAffected()
                    << "library purged tracks from crates";
        }
    }
}


void CrateStorage::connectDatabase(QSqlDatabase database) {
    m_database = database;
    createViews();
}


void CrateStorage::disconnectDatabase() {
    // Ensure that we don't use the current database connection
    // any longer.
    m_database = QSqlDatabase();
}


void CrateStorage::createViews() {
    VERIFY_OR_DEBUG_ASSERT(FwdSqlQuery(m_database, kCrateSummaryViewQuery).execPrepared()) {
        kLogger.critical()
                << "Failed to create database view for crate summaries!";
    }
}


uint CrateStorage::countCrates() const {
    FwdSqlQuery query(m_database, QString(
            "SELECT COUNT(*) FROM %1").arg(
                    CRATE_TABLE));
    if (query.execPrepared() && query.next()) {
        uint result = query.fieldValue(0).toUInt();
        DEBUG_ASSERT(!query.next());
        return result;
    } else {
        return 0;
    }
}


bool CrateStorage::readCrateById(CrateId id, Crate* pCrate) const {
    FwdSqlQuery query(m_database, QString(
            "SELECT * FROM %1 WHERE %2=:id").arg(
                    CRATE_TABLE,
                    CRATETABLE_ID));
    query.bindValue(":id", id);
    if (query.execPrepared()) {
        CrateSelectResult crates(std::move(query));
        if ((pCrate != nullptr) ? crates.populateNext(pCrate) : crates.next()) {
            VERIFY_OR_DEBUG_ASSERT(!crates.next()) {
                kLogger.warning()
                        << "Ambiguous crate id:" << id;
            }
            return true;
        } else {
            kLogger.warning()
                    << "Crate not found by id:" << id;
        }
    }
    return false;
}


bool CrateStorage::readCrateByName(const QString& name, Crate* pCrate) const {
    FwdSqlQuery query(m_database, QString(
            "SELECT * FROM %1 WHERE %2=:name").arg(
                    CRATE_TABLE,
                    CRATETABLE_NAME));
    query.bindValue(":name", name);
    if (query.execPrepared()) {
        CrateSelectResult crates(std::move(query));
        if ((pCrate != nullptr) ? crates.populateNext(pCrate) : crates.next()) {
            VERIFY_OR_DEBUG_ASSERT(!crates.next()) {
                kLogger.warning()
                        << "Ambiguous crate name:" << name;
            }
            return true;
        } else {
            kLogger.debug()
                    << "Crate not found by name:" << name;
        }
    }
    return false;
}


CrateSelectResult CrateStorage::selectCrates() const {
    FwdSqlQuery query(m_database,
            mixxx::DbConnection::collateLexicographically(QString(
                    "SELECT * FROM %1 ORDER BY %2").arg(
                            CRATE_TABLE,
                            CRATETABLE_NAME)));

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
            mixxx::DbConnection::collateLexicographically(QString(
                    "SELECT * FROM %1 WHERE %2 %3 (%4) ORDER BY %5").arg(
                            CRATE_TABLE,
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
            mixxx::DbConnection::collateLexicographically(QString(
                    "SELECT * FROM %1 WHERE %2=:autoDjSource ORDER BY %3").arg(
                            CRATE_TABLE,
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
            mixxx::DbConnection::collateLexicographically(QString(
                    "SELECT * FROM %1 ORDER BY %2").arg(
                            CRATE_SUMMARY_VIEW,
                            CRATETABLE_NAME)));
    if (query.execPrepared()) {
        return CrateSummarySelectResult(std::move(query));
    } else {
        return CrateSummarySelectResult();
    }
}

bool CrateStorage::readCrateSummaryById(CrateId id, CrateSummary* pCrateSummary) const {
    FwdSqlQuery query(m_database, QString(
            "SELECT * FROM %1 WHERE %2=:id").arg(
                    CRATE_SUMMARY_VIEW,
                    CRATETABLE_ID));
    query.bindValue(":id", id);
    if (query.execPrepared()) {
        CrateSummarySelectResult crateSummaries(std::move(query));
        if ((pCrateSummary != nullptr) ? crateSummaries.populateNext(pCrateSummary) : crateSummaries.next()) {
            VERIFY_OR_DEBUG_ASSERT(!crateSummaries.next()) {
                kLogger.warning()
                        << "Ambiguous crate id:" << id;
            }
            return true;
        } else {
            kLogger.warning()
                    << "Crate summary not found by id:" << id;
        }
    }
    return false;
}


uint CrateStorage::countCrateTracks(CrateId crateId) const {
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
QString CrateStorage::formatSubselectQueryForCrateTrackIds(
        CrateId crateId) {
    return QString("SELECT %1 FROM %2 WHERE %3=%4").arg(
            CRATETRACKSTABLE_TRACKID,
            CRATE_TRACKS_TABLE,
            CRATETRACKSTABLE_CRATEID,
            crateId.toString());
}


QString CrateStorage::formatQueryForTrackIdsByCrateNameLike (
        const QString& crateNameLike) const {
    FieldEscaper escaper(m_database);
    QString escapedArgument = escaper.escapeString(kSqlLikeMatchAll + crateNameLike + kSqlLikeMatchAll);

    return QString(
        "SELECT DISTINCT %1 FROM %2 JOIN %3 ON %4=%5 WHERE %6 LIKE %7 ORDER BY %1").arg(
            CRATETRACKSTABLE_TRACKID,
            CRATE_TRACKS_TABLE,
            CRATE_TABLE,
            CRATETRACKSTABLE_CRATEID,
            CRATETABLE_ID,
            CRATETABLE_NAME,
            escapedArgument);
}


CrateTrackSelectResult CrateStorage::selectCrateTracksSorted(CrateId crateId) const {
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


CrateTrackSelectResult CrateStorage::selectTrackCratesSorted(TrackId trackId) const {
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


CrateTrackSelectResult CrateStorage::selectTracksSortedByCrateNameLike(const QString& crateNameLike) const {
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


QSet<CrateId> CrateStorage::collectCrateIdsOfTracks(const QList<TrackId>& trackIds) const {
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


bool CrateStorage::onInsertingCrate(
        const Crate& crate,
        CrateId* pCrateId) {
    VERIFY_OR_DEBUG_ASSERT(!crate.getId().isValid()) {
        kLogger.warning()
                << "Cannot insert crate with a valid id:" << crate.getId();
        return false;
    }
    FwdSqlQuery query(m_database, QString(
            "INSERT INTO %1 (%2,%3,%4) VALUES (:name,:locked,:autoDjSource)").arg(
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
    FwdSqlQuery query(m_database, QString(
            "UPDATE %1 SET %2=:name,%3=:locked,%4=:autoDjSource WHERE %5=:id").arg(
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
        FwdSqlQuery query(m_database, QString(
                "DELETE FROM %1 WHERE %2=:id").arg(
                        CRATE_TRACKS_TABLE,
                        CRATETRACKSTABLE_CRATEID));
        VERIFY_OR_DEBUG_ASSERT(query.isPrepared()) {
            return false;
        }
        query.bindValue(":id", crateId);
        VERIFY_OR_DEBUG_ASSERT(query.execPrepared()) {
            return false;
        }
        if (query.numRowsAffected() <= 0) {
            kLogger.debug()
                    << "Deleting empty crate with id" << crateId;
        }
    }
    {
        FwdSqlQuery query(m_database, QString(
                "DELETE FROM %1 WHERE %2=:id").arg(
                        CRATE_TABLE,
                        CRATETABLE_ID));
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


bool CrateStorage::onRemovingCrateTracks(
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


bool CrateStorage::onPurgingTracks(
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


uint CrateStorage::countCratesInClosure() const {
    FwdSqlQuery query(
      m_database, QString(
        "SELECT COUNT(*) FROM %1 "
        "WHERE %2 = %3").arg(
          CRATE_CLOSURE_TABLE,
          CLOSURE_PARENTID,
          CLOSURE_CHILDID));
    if (query.execPrepared() && query.next()) {
        uint result = query.fieldValue(0).toUInt();
        DEBUG_ASSERT(!query.next());
        return result;
    } else {
        return 0;
    }
}

void CrateStorage::checkClosure() const {
    if (countCratesInClosure() == countCrates()) {
        resetClosure();
        initClosure();
        resetPath();
        generateAllPaths();
    }
}

void CrateStorage::resetClosure() const {
    FwdSqlQuery query(
      m_database, QString(
        "DELETE FROM %1").arg(
          CRATE_CLOSURE_TABLE));
    if (!query.isPrepared()) {
        return;
    }
    if (!query.execPrepared()) {
        return;
    }
}


bool CrateStorage::initClosure() const {
    std::vector<CrateId> crateIds;

    CrateSelectResult crates(selectCrates());
    Crate crate;

    while (crates.populateNext(&crate)) {
        crateIds.push_back(crate.getId());
    }

    FwdSqlQuery query(
      m_database, QString(
        "INSERT INTO %1 VALUES("
        ":parent, :child, 0)").arg(
          CRATE_CLOSURE_TABLE));
    if (!query.isPrepared()) {
        return false;
    }

    while (crateIds.size() != 0) {
        query.bindValue(":parent", crateIds.back());
        query.bindValue(":child", crateIds.back());
        if (!query.execPrepared()) {
            return false;
        }
        crateIds.pop_back();
    }

    return true;
}

void CrateStorage::resetPath() const {
    FwdSqlQuery query(
      m_database, QString(
        "DELETE FROM %1").arg(
          CRATE_PATH_TABLE));
    if (!query.isPrepared()) {
        return;
    }
    if (!query.execPrepared()) {
        return;
    }
}

bool CrateStorage::writeCratePaths(CrateId id, QString namePath, QString idPath) const {
    FwdSqlQuery query(
      m_database, QString(
        "INSERT INTO %1 "
        "VALUES (:id, :idPath, :namePath)").arg(
          CRATE_PATH_TABLE));
    if (!query.isPrepared()) {
        return false;
    }
    query.bindValue(":id", id);
    query.bindValue(":namePath", namePath);
    query.bindValue(":idPath", idPath);
    if (!query.execPrepared()) {
        return false;
    }

    return true;
}

bool CrateStorage::generateCratePaths(Crate crate) const {
    QSqlQuery query(m_database);

    query.prepare(
      QString(
        "SELECT p.%1, p.%2 FROM %3 "
        "JOIN %4 p ON %5 = p.%2 "
        "JOIN %4 c ON %6 = c.%2 "
        "where c.%1 = :childName and %7 != 0 "
        "ORDER BY %7 DESC").arg(
          CRATETABLE_NAME,
          CRATETABLE_ID,
          CRATE_CLOSURE_TABLE,
          CRATE_TABLE,
          CLOSURE_PARENTID,
          CLOSURE_CHILDID,
          CLOSURE_DEPTH));

    query.bindValue(":childName", crate.getName());
    query.setForwardOnly(true);
    QString namePath;
    QString idPath;

    if (query.exec()) {
        while (query.next()) {
            namePath = namePath + "/" + query.value(0).toString();
            idPath = idPath + "/" + query.value(1).toString();
        }
    } else {
        return false;
    }

    namePath = namePath + "/" + crate.getName();
    idPath = idPath + "/" + crate.getId().toString();

    return writeCratePaths(crate.getId(), namePath, idPath);
}

bool CrateStorage::generateAllPaths() const {
    CrateSelectResult crates(selectCrates());
    Crate crate;
    while (crates.populateNext(&crate)) {
        generateCratePaths(crate);
    }

    return true;
}

bool CrateStorage::findParentAndChildFromPath(Crate& parent,
                                                Crate& child,
                                                const QString& idPath) const {
    QStringList ids = idPath.split("/", QString::SkipEmptyParts);

    // get the last item (child)
    readCrateById(CrateId(ids.back()), &child);
    if (ids.size() > 1) {
        // get the second to last item (parent)
        readCrateById(CrateId(ids.at(ids.size() - 2)), &parent);
    } else {
        // if there isn't one return false
        return false;
    }
    return true;
}

bool CrateStorage::initClosureForCrate(CrateId id) const {
    FwdSqlQuery query(
      m_database, QString(
        "INSERT INTO %1 "
        "VALUES(:parent, :child, 0)").arg(
          CRATE_CLOSURE_TABLE));

    if (!query.isPrepared()) {
        return false;
    }

    // closure dependacy
    query.bindValue(":parent", id);
    query.bindValue(":child", id);
    if (!query.execPrepared()) {
        return false;
    }

    return true;
}

bool CrateStorage::insertIntoClosure(CrateId parent, CrateId child) const {
    FwdSqlQuery query(
      m_database, QString(
        "INSERT INTO %1(%2, %3, %4) "
        "SELECT p.%2, c.%3, p.%4 + c.%4 + 1 "
        "FROM %1 p, %1 c "
        "WHERE p.%3 = :parent AND c.%2 = :child").arg(
          CRATE_CLOSURE_TABLE,
          CLOSURE_PARENTID,
          CLOSURE_CHILDID,
          CLOSURE_DEPTH));

    if (!query.isPrepared()) {
        return false;
    }

    query.bindValue(":parent", parent);
    query.bindValue(":child", child);

    if (!query.execPrepared()) {
        return false;
    }

    return true;
}

void CrateStorage::deleteCrate(CrateId id) const {
    // TODO(gramanas) cratedeletion from the hierarchy must
    // be smart (delete crate with children)
    FwdSqlQuery query(
      m_database, QString(
        "DELETE FROM cratePath WHERE crateId = :id"));
    query.bindValue(":id", id);
    if (!query.isPrepared()) {
        return;
    }
    if (!query.execPrepared()) {
        return;
    }

    FwdSqlQuery query2(m_database, QString(
        "DELETE FROM crateClosure WHERE childId = :id"));
    query2.bindValue(":id", id);
    if (!query2.isPrepared()) {
        return;
    }
    if (!query2.execPrepared()) {
        return;
    }
}

bool CrateStorage::hasChildern(CrateId id) const {
    FwdSqlQuery query(
      m_database, QString(
        "SELECT COUNT(*) FROM %1 "
        "WHERE %2 = :id AND %3 != 0").arg(
          CRATE_CLOSURE_TABLE,
          CLOSURE_PARENTID,
          CLOSURE_DEPTH));

    query.bindValue(":id", id);
    if (query.execPrepared() && query.next()) {
        return query.fieldValue(0).toUInt() != 0;
    }
    return false;
}

QStringList CrateStorage::collectIdPaths() const {
    QSqlQuery query(m_database);

    query.prepare(
      QString(
        "SELECT %1 FROM %2 "
        "ORDER BY %3").arg(
          PATHTABLE_ID_PATH,
          CRATE_PATH_TABLE,
          PATHTABLE_NAME_PATH));

    query.setForwardOnly(true);

    QString idPath;
    QStringList idPaths = {};

    if (query.exec()) {
        while (query.next()) {
            idPath = query.value(0).toString();
            idPaths.append(idPath);
        }
    } else {
        return idPaths; //returns empty list if it fails
    }

    return idPaths;
}

QStringList CrateStorage::tokenizeCratePath(CrateId id) const {
    FwdSqlQuery query(
      m_database, QString(
        "SELECT %1 FROM %2 "
        "WHERE %3 = :id").arg(
          PATHTABLE_NAME_PATH,
          CRATE_PATH_TABLE,
          PATHTABLE_CRATEID));

    query.bindValue(":id", id);
    if (query.execPrepared() && query.next()) {
            return query.fieldValue(0).toString().
                split("/", QString::SkipEmptyParts);
    }

    return QStringList();
}

QStringList CrateStorage::collectRootCrates() const {
    QStringList names;

    FwdSqlQuery query(
      m_database, QString(
        "SELECT %1 FROM %2 "
        "JOIN %3 ON %4 = %5 "
        "GROUP BY %5 "
        "HAVING COUNT(*) = 1").arg(
          CRATETABLE_NAME,
          CRATE_CLOSURE_TABLE,
          CRATE_TABLE,
          CRATETABLE_ID,
          CLOSURE_CHILDID));

    if (query.execPrepared())
        while (query.next()) {
            names << query.fieldValue(0).toString();
    }

    return names;
}
