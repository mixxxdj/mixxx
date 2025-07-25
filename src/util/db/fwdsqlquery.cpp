#include "util/db/fwdsqlquery.h"

#include "util/performancetimer.h"
#include "util/logger.h"
#include "util/assert.h"

namespace {

const mixxx::Logger kLogger("FwdSqlQuery");

bool prepareQuery(QSqlQuery& query, const QString& statement) {
    DEBUG_ASSERT(!query.isActive());
    query.setForwardOnly(true);
    PerformanceTimer timer;
    if (kLogger.traceEnabled()) {
        timer.start();
    }
    if (query.prepare(statement)) {
        if (kLogger.traceEnabled()) {
            kLogger.tracePerformance(
                    QString("Preparing \"%1\"").arg(statement),
                    timer);
        }
        return true;
    } else {
        return false;
    }
}

} // anonymous namespace

FwdSqlQuery::FwdSqlQuery(
        const QSqlDatabase& database,
        const QString& statement)
        : QSqlQuery(database),
          m_prepared(prepareQuery(*this, statement)) {
    if (!m_prepared) {
        DEBUG_ASSERT(!database.isOpen() || hasError());
        if (hasDuplicateColumnNameError()) {
            // Special case: Errors caused by re-adding a column when
            // re-appliying a schema migration are expected and we
            // should not report them as an error.
            kLogger.debug()
                    << "Failed to prepare schema migration statement"
                    << statement
                    << ":"
                    << lastError();
        } else {
            kLogger.critical()
                    << "Failed to prepare statement"
                    << statement
                    << ":"
                    << lastError();
        }
    }
}

bool FwdSqlQuery::hasDuplicateColumnNameError() const {
    return hasError() &&
            lastError().databaseText().startsWith(
                    QStringLiteral("duplicate column name: "));
}

bool FwdSqlQuery::execPrepared() {
    DEBUG_ASSERT(isPrepared());
    DEBUG_ASSERT(!hasError());
    PerformanceTimer timer;
    if (kLogger.traceEnabled()) {
        timer.start();
    }
    if (exec()) {
        if (kLogger.traceEnabled()) {
            if (kLogger.traceEnabled()) {
                kLogger.tracePerformance(
                        QString("Executing \"%1\"").arg(executedQuery()),
                        timer);
            }
        }
        DEBUG_ASSERT(!hasError());
        // Verify our assumption that the size of the result set
        // is unavailable for forward-only queries. Otherwise we
        // should add the size() operation from QSqlQuery.
        DEBUG_ASSERT(size() < 0);
        return true;
    } else {
        kLogger.warning()
                << "Failed to execute"
                << lastQuery()
                << ":"
                << lastError();
        DEBUG_ASSERT(hasError());
        return false;
    }
}

DbFieldIndex FwdSqlQuery::fieldIndex(const QString& fieldName) const {
    DEBUG_ASSERT(!hasError());
    DEBUG_ASSERT(isSelect());
    DbFieldIndex fieldIndex(record().indexOf(fieldName));
    VERIFY_OR_DEBUG_ASSERT(fieldIndex.isValid()) {
        kLogger.critical()
                << "Field named" << fieldName
                << "not found in result from"
                << executedQuery();
    }
    DEBUG_ASSERT(!hasError());
    return fieldIndex;
}

namespace {

constexpr int kBooleanFalse = 0;
constexpr int kBooleanTrue = 1;

// NOTE(uklotzde): This conversion has been wrapped into a separate
// function, because the conversion is completely independent of the
// query, the current record and how values for individual fields are
// retrieved. For separation of concerns and to improve readability.
// Please do not try to unwrap the code! It is already declared "inline" ;)
//
// Recommended reading: "Refactoring" by Martin Fowler
inline bool boolFromQVariantInt(const QVariant& variant) {
    bool ok = false;
    int value = variant.toInt(&ok);
    VERIFY_OR_DEBUG_ASSERT(ok) {
        kLogger.critical()
                << "Invalid boolean value in database:"
                << variant;
    }
    VERIFY_OR_DEBUG_ASSERT(
            (value == kBooleanFalse) ||
            (value == kBooleanTrue)) {
        kLogger.critical()
                << "Invalid boolean value in database:"
                << value;
    }
    // C-style conversion from int to bool
    static_assert(kBooleanFalse == 0);
    return value != kBooleanFalse;
}

} // anonymous namespace

bool FwdSqlQuery::fieldValueBoolean(DbFieldIndex fieldIndex) const {
    return boolFromQVariantInt(fieldValue(fieldIndex));
}
