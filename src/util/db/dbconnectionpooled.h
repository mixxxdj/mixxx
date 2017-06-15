#ifndef MIXXX_DBCONNECTIONPOOLED_H
#define MIXXX_DBCONNECTIONPOOLED_H


#include <QSqlDatabase>

#include "util/db/dbconnectionpool.h"


namespace mixxx {

// Manages the lifetime of a thread-local database connection that is
// shared through DbConnectionPool. It is created and added to the pool
// upon construction and will be closed and removed from the pool upon
// destruction.
//
// Ultimately upon termination of a thread the corresponding connection
// would also be closed and removed implicitly by the pool, but that
// should never happen! Therefore this class should always be allocated
// on the stack and not dynamically on the heap so that it cannot outlive
// the corresponding thread.
class DbConnectionPooled final {
  public:
    explicit DbConnectionPooled(
            DbConnectionPoolPtr pDbConnectionPool = DbConnectionPoolPtr());
    DbConnectionPooled(
            DbConnectionPooled&& other) = default;
    ~DbConnectionPooled();

    // Checks if a thread-local connection has actually been created during
    // construction. Otherwise this instance is non-functional and dead.
    operator bool() const {
        return m_pDbConnectionPool != nullptr;
    }

    QString name() const {
        return m_sqlDatabase.connectionName();
    }

    bool isOpen() const {
        return m_sqlDatabase.isOpen();
    }

    // The thread-local connection that has been created during construction.
    operator QSqlDatabase() const {
        return m_sqlDatabase;
    }

    friend QDebug operator<<(QDebug debug, const DbConnectionPooled& connection);

  private:
    DbConnectionPooled(const DbConnectionPooled&) = delete;
    DbConnectionPooled& operator=(const DbConnectionPooled&) = delete;
    DbConnectionPooled& operator=(DbConnectionPooled&&) = delete;

    // Prevent heap allocation
    static void * operator new(std::size_t);
    static void * operator new[](std::size_t);

    DbConnectionPoolPtr m_pDbConnectionPool;
    QSqlDatabase m_sqlDatabase;
};

} // namespace mixxx


#endif // MIXXX_DBCONNECTIONPOOLED_H
