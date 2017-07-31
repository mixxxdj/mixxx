#include "library/features/crates/cratemanager.h"

#include "util/assert.h"
#include "util/db/sqltransaction.h"
#include "util/logger.h"

namespace {

const mixxx::Logger kLogger("CrateStorage");

} // anonymus namespace


void CrateManager::repairDatabase(QSqlDatabase database) {
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
        FwdSqlQuery query(
          database, QString(
            "DELETE FROM %1 "
            "WHERE %2 IS NULL "
            "OR TRIM(%2)=''").arg(
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
        FwdSqlQuery query(
          database, QString(
            "UPDATE %1 SET %2=0 "
            "WHERE %2 NOT IN (0,1)").arg(
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
        FwdSqlQuery query(
          database, QString(
            "UPDATE %1 SET %2=0 "
            "WHERE %2 NOT IN (0,1)").arg(
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
        FwdSqlQuery query(
          database, QString(
            "DELETE FROM %1 "
            "WHERE %2 NOT IN "
            "(SELECT %3 FROM %4)").arg(
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
        FwdSqlQuery query(
          database, QString(
            "DELETE FROM %1 "
            "WHERE %2 NOT IN "
            "(SELECT %3 FROM %4)").arg(
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

    // Crate hierarchy
    {
        // Remove paths from non-existent crates
        FwdSqlQuery query(
          database, QString(
            "DELETE FROM %1 "
            "WHERE %2 NOT IN "
            "(SELECT %3 FROM $4)").arg(
              CRATE_PATH_TABLE,
              PATHTABLE_CRATEID,
              CRATE_TABLE,
              CRATETABLE_ID));
        if (query.execPrepared() && (query.numRowsAffected() > 0)) {
            kLogger.warning()
                << "Removed" << query.numRowsAffected()
                << "paths from non-existent crates";
        }
    }
}

void CrateManager::connectDatabase(QSqlDatabase database) {
    m_database = database;

    m_crateStorage.initialize(database);
    m_crateTracks.initialize(database);
    m_crateHierarchy.initialize(database);

    createViews();
}

void CrateManager::disconnectDatabase() {
    // Ensure that we don't use the current database connection
    // any longer.
    m_database = QSqlDatabase();
}

bool CrateManager::onPurgingTracks(const QList<TrackId> &trackIds) {
    m_crateTracks.onPurgingTracks(trackIds);
}

void CrateManager::checkClosure() {
    if (m_crateHierarchy.countCratesInClosure() != m_crateStorage.countCrates()) {
        m_crateHierarchy.reset(&storage());
    }
}

bool CrateManager::insertCrate(
        const Crate& crate,
        CrateId* pCrateId) {
    // Transactional
    SqlTransaction transaction(m_database);
    VERIFY_OR_DEBUG_ASSERT(transaction) {
        return false;
    }
    CrateId crateId;
    VERIFY_OR_DEBUG_ASSERT(m_crateStorage.onInsertingCrate(crate, &crateId)) {
        return false;
    }
    DEBUG_ASSERT(crateId.isValid());
    VERIFY_OR_DEBUG_ASSERT(transaction.commit()) {
        return false;
    }

    // Emit signals
    emit(crateInserted(crateId));

    if (pCrateId != nullptr) {
        *pCrateId = crateId;
    }
    return true;
}

bool CrateManager::updateCrate(
        const Crate& crate) {
    // Transactional
    SqlTransaction transaction(m_database);
    VERIFY_OR_DEBUG_ASSERT(transaction) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(m_crateStorage.onUpdatingCrate(crate)) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(m_crateHierarchy.onUpdatingCrate(crate, &storage())) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(transaction.commit()) {
        return false;
    }

    // Emit signals
    emit(crateUpdated(crate.getId()));

    return true;
}

bool CrateManager::deleteCrate(
        CrateId crateId) {
    // Transactional
    SqlTransaction transaction(m_database);
    VERIFY_OR_DEBUG_ASSERT(transaction) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(m_crateStorage.onDeletingCrate(crateId)) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(transaction.commit()) {
        return false;
    }

    // Emit signals
    emit(crateDeleted(crateId));

    return true;
}

bool CrateManager::addTracksToCrate(
        CrateId crateId,
        const QList<TrackId>& trackIds) {
    // Transactional
    SqlTransaction transaction(m_database);
    VERIFY_OR_DEBUG_ASSERT(transaction) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(m_crateTracks.onAddingCrateTracks(crateId, trackIds)) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(transaction.commit()) {
        return false;
    }

    // Emit signals
    emit(crateTracksChanged(crateId, trackIds, QList<TrackId>()));

    return true;
}

bool CrateManager::removeTracksFromCrate(
        CrateId crateId,
        const QList<TrackId>& trackIds) {
    // Transactional
    SqlTransaction transaction(m_database);
    VERIFY_OR_DEBUG_ASSERT(transaction) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(m_crateTracks.onRemovingCrateTracks(crateId, trackIds)) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(transaction.commit()) {
        return false;
    }

    // Emit signals
    emit(crateTracksChanged(crateId, QList<TrackId>(), trackIds));

    return true;
}

bool CrateManager::updateAutoDjCrate(
        CrateId crateId,
        bool isAutoDjSource) {
    Crate crate;
    VERIFY_OR_DEBUG_ASSERT(m_crateStorage.readCrateById(crateId, &crate)) {
        return false; // nonexistent or failure
    }
    if (crate.isAutoDjSource() == isAutoDjSource) {
        return false; // nothing to do
    }
    crate.setAutoDjSource(isAutoDjSource);
    return updateCrate(crate);
}

void CrateManager::createViews() {
    VERIFY_OR_DEBUG_ASSERT(FwdSqlQuery(m_database, kCrateSummaryViewQuery).execPrepared()) {
        kLogger.critical()
                << "Failed to create database view for crate summaries!";
    }
}
