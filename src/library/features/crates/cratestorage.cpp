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

} // anonymous namespace

void CrateStorage::initialize(const QSqlDatabase& database) {
    m_database = database;
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
