#ifndef MIXXX_DBCONNECTIONPOOLED_H
#define MIXXX_DBCONNECTIONPOOLED_H


#include <QSqlDatabase>

#include "util/db/dbconnectionpool.h"
#include "util/db/dbconnectionpooler.h"


namespace mixxx {

// Dynamically provides thread-local database connections from
// the pool.
class DbConnectionPooled final {
  public:
    explicit DbConnectionPooled(
            DbConnectionPoolPtr pDbConnectionPool = DbConnectionPoolPtr())
        : m_pDbConnectionPool(std::move(pDbConnectionPool)) {
    }
    explicit DbConnectionPooled(
            const DbConnectionPooler& pDbConnectionPooler)
        : m_pDbConnectionPool(pDbConnectionPooler.m_pDbConnectionPool) {
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
    // constructed database commection is returned.
    explicit operator QSqlDatabase() const;

  private:
    DbConnectionPoolPtr m_pDbConnectionPool;
};

} // namespace mixxx


#endif // MIXXX_DBCONNECTIONPOOLED_H
