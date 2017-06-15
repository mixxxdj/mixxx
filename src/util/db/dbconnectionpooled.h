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

    explicit operator bool() const {
        return static_cast<bool>(m_pDbConnectionPool);
    }

    explicit operator QSqlDatabase() const;

  private:
    DbConnectionPoolPtr m_pDbConnectionPool;
};

} // namespace mixxx


#endif // MIXXX_DBCONNECTIONPOOLED_H
