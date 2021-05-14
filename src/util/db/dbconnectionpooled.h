#pragma once

#include <QSqlDatabase>

#include "util/db/dbconnectionpool.h"


namespace mixxx {

// Dynamically provides thread-local database connections from
// the pool.
class DbConnectionPooled final {
  public:
    explicit DbConnectionPooled(
            DbConnectionPoolPtr pDbConnectionPool = DbConnectionPoolPtr())
        : m_pDbConnectionPool(std::move(pDbConnectionPool)) {
    }

    // Checks if this instance actually references a connection pool
    // needed for obtaining thread-local database connections (see below).
    explicit operator bool() const {
        return static_cast<bool>(m_pDbConnectionPool);
    }

    // Tries to obtain an existing thread-local database connection
    // from the pool. This might fail if either the reference to the
    // connection pool is missing or if the pool does not contain a
    // thread-local connection for this thread (previously created
    // by some DbConnectionPooler). On failure a non-functional default
    // constructed database connection is returned.
    //
    // The returned connections is not bound to this instance:
    // QSqlDatabase dbConnection = DbConnectionPooled(...);
    /*implicit*/ operator QSqlDatabase() const;

  private:
    DbConnectionPoolPtr m_pDbConnectionPool;
};

} // namespace mixxx
