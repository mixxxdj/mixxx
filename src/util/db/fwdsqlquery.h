#pragma once


#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>

#include "util/db/dbid.h"
#include "util/db/dbfieldindex.h"

#include "util/assert.h"

// forward declarations
class SqlQueryFinisher;
class FwdSqlQuerySelectResult;


// A forward-only QSqlQuery that is prepared immediately
// during initialization. It offers a limited set of functions
// from QSqlQuery.
//
// Setting QSqlQuery to forward-only causes memory savings since
// QSqlCachedResult (what QtSQLite uses) won't allocate a giant
// in-memory table that we won't use at all when invoking only
// QSqlQuery::next() to iterate over the results.
//
// Prefer to use this derived class instead of QSqlQuery to avoid
// performance bottlenecks and for implicit logging failed query
// executions.
//
// Please note that forward-only queries don't provide information
// about the size of the result set!
class FwdSqlQuery: protected QSqlQuery {
    friend class SqlQueryFinisher;
    friend class FwdSqlQuerySelectResult;

  public:
    FwdSqlQuery(
            const QSqlDatabase& database,
            const QString& statement);

    bool isPrepared() const {
        return m_prepared;
    }

    bool hasError() const {
        return lastError().isValid() &&
                (lastError().type() != QSqlError::NoError);
    }
    QSqlError lastError() const {
        return QSqlQuery::lastError();
    }

    void bindValue(const QString& placeholder, const QVariant& value) {
        QSqlQuery::bindValue(placeholder, value);
    }

    // Overloaded function for type DbId
    void bindValue(const QString& placeholder, const DbId& value) {
        bindValue(placeholder, value.toVariant());
    }

    QString executedQuery() const {
        return QSqlQuery::executedQuery();
    }

    // Execute the prepared query and log errors on failure.
    //
    // Please note, that the member function exec() inherited
    // from the base class QSqlQuery is not polymorphic (virtual)
    // and can't be overridden safely!
    bool execPrepared();

    QSqlRecord record() const {
        return QSqlQuery::record();
    }

    int numRowsAffected() const {
        DEBUG_ASSERT(!hasError());
        return QSqlQuery::numRowsAffected();
    }

    QVariant lastInsertId() const {
        DEBUG_ASSERT(!lastError().isValid());
        DEBUG_ASSERT(!isSelect());
        QVariant result(QSqlQuery::lastInsertId());
        DEBUG_ASSERT(result.isValid());
        DEBUG_ASSERT(!result.isNull());
        return result;
    }

    bool next() {
        DEBUG_ASSERT(!hasError());
        DEBUG_ASSERT(isSelect());
        return QSqlQuery::next();
    }

    DbFieldIndex fieldIndex(const QString& fieldName) const;

    QVariant fieldValue(DbFieldIndex fieldIndex) const {
        DEBUG_ASSERT(!hasError());
        DEBUG_ASSERT(isSelect());
        return value(fieldIndex);
    }

    bool fieldValueBoolean(DbFieldIndex fieldIndex) const;

  private:
    FwdSqlQuery() = default; // hidden

    bool m_prepared;
};
