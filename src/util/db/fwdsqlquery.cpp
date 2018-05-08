#include "util/db/fwdsqlquery.h"

#include <QSqlRecord>

#include <QtDebug>

#include "util/assert.h"


namespace {

inline
bool prepareQuery(QSqlQuery& query, const QString& statement) {
    DEBUG_ASSERT(!query.isActive());
    query.setForwardOnly(true);
    return query.prepare(statement);
}

} // anonymous namespace

FwdSqlQuery::FwdSqlQuery(
        QSqlDatabase database,
        const QString& statement)
    : QSqlQuery(database),
      m_prepared(prepareQuery(*this, statement)) {
    if (!m_prepared) {
        DEBUG_ASSERT(!database.isOpen() || hasError());
        qCritical() << "Failed to prepare SQL query"
                << "for [" << statement << "]"
                << "on [" << database.connectionName() << "]:"
                << lastError();
    }
}

bool FwdSqlQuery::execPrepared() {
    DEBUG_ASSERT(isPrepared());
    DEBUG_ASSERT(!hasError());
    if (exec()) {
        DEBUG_ASSERT(!hasError());
        // Verify our assumption that the size of the result set
        // is unavailable for forward-only queries. Otherwise we
        // should add the size() operation from QSqlQuery.
        DEBUG_ASSERT(size() < 0);
        return true;
    } else {
        if (lastQuery() == executedQuery()) {
            qCritical() << "Failed to execute prepared SQL query"
                    << "lastQuery [" << lastQuery() << "]:"
                    << lastError();
        } else {
            qCritical() << "Failed to execute prepared SQL query"
                    << "lastQuery [" << lastQuery() << "]"
                    << "executedQuery [" << executedQuery() << "]:"
                    << lastError();
        }
        DEBUG_ASSERT(hasError());
        return false;
    }
}

DbFieldIndex FwdSqlQuery::fieldIndex(const QString& fieldName) const {
    DEBUG_ASSERT(!hasError());
    DEBUG_ASSERT(isSelect());
    DbFieldIndex fieldIndex(record().indexOf(fieldName));
    VERIFY_OR_DEBUG_ASSERT(fieldIndex.isValid()) {
        qCritical() << "Field named"
                << fieldName
                << "not found in record of SQL query"
                << executedQuery();
    }
    DEBUG_ASSERT(!hasError());
    return fieldIndex;
}

namespace {
    // NOTE(uklotzde): This conversion has been wrapped into a separate
    // function, because the conversion is completely independent of the
    // query, the current record and how values for individual fields are
    // retrieved. For separation of concerns and to improve readability.
    // Please do not try to unwrap the code! It is already declared "inline" ;)
    //
    // Recommended reading: "Refactoring" by Martin Fowler
    inline
    bool toBoolean(const QVariant& variant) {
        bool ok = false;
        int value = variant.toInt(&ok);
        VERIFY_OR_DEBUG_ASSERT(ok) {
            qWarning() << "Invalid boolean value in database:" << variant;
        }
        VERIFY_OR_DEBUG_ASSERT(
                (value == FwdSqlQuery::BOOLEAN_FALSE) ||
                (value == FwdSqlQuery::BOOLEAN_TRUE)) {
            qWarning() << "Invalid boolean value in database:" << value;
        }
        // C-style conversion from int to bool
        DEBUG_ASSERT(FwdSqlQuery::BOOLEAN_FALSE == 0);
        return value != FwdSqlQuery::BOOLEAN_FALSE;
    }
} // anonymous namespace

bool FwdSqlQuery::fieldValueBoolean(DbFieldIndex fieldIndex) const {
    return toBoolean(fieldValue(fieldIndex));
}
