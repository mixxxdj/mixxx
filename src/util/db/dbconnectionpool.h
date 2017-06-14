#ifndef MIXXX_DBCONNECTIONPOOL_H
#define MIXXX_DBCONNECTIONPOOL_H


#include <QAtomicInt>
#include <QThreadStorage>

#include "util/db/dbconnection.h"
#include "util/memory.h"
#include "util/assert.h"


namespace mixxx {

class DbConnectionPool;
typedef std::shared_ptr<DbConnectionPool> DbConnectionPoolPtr;

class DbConnectionPool final {
  public:
    // Creates a new pool of database connections (one per thread) that
    // all use the same connection parameters. Unique connection names
    // will be  generated based on the given connection name that serves
    // as the base name (= common prefix).
    static DbConnectionPoolPtr create(
            const DbConnection::Params& params,
            const QString& connectionName) {
        return std::make_shared<DbConnectionPool>(params, connectionName);
    }

    // NOTE(uklotzde): Should be private, but must be public for invocation
    // from std::make_shared()!
    DbConnectionPool(
            const DbConnection::Params& params,
            const QString& connectionName); // reserver for

    // Returns a database connection for the current thread, that has
    // previously been created by instantiating ThreadLocalScoped. The
    // returned connection is only valid within the current thread! It
    // will be closed and removed from the pool upon the destruction of
    // the owning ThreadLocalScoped (see below) or as a very last resort
    // implicitly when the current thread terminates. Since all connections
    // need to be created through ThreadLocalScoped the latter case should
    // never happen.
    QSqlDatabase threadLocalConnection() const;

    // Temporarily creates and destroys a thread-local database connection
    // that exists for the lifetime of this instance, which effectively is
    // its owner. The connection will be closed and removed from the pool upon
    // destruction, i.e. maybe long before the corresponding thread is actually
    // terminated. Upon termination of a thread the corresponding connection
    // would also be closed and removed implicitly, but that should never happen.
    class ThreadLocalScoped final {
      public:
        explicit ThreadLocalScoped(
                DbConnectionPoolPtr pDbConnectionPool = DbConnectionPoolPtr());
        ThreadLocalScoped(
                ThreadLocalScoped&& other) = default;
        ~ThreadLocalScoped();

        // Checks if a thread-local connection has actually been created during
        // construction. Otherwise this instance is non-functional and dead.
        operator bool() const {
            return m_pDbConnectionPool != nullptr;
        }

        // The thread-local connection that has been created during construction.
        operator QSqlDatabase() const {
            return m_dbConnection;
        }

      private:
        ThreadLocalScoped(const ThreadLocalScoped&) = delete;
        ThreadLocalScoped& operator=(const ThreadLocalScoped&) = delete;
        ThreadLocalScoped& operator=(ThreadLocalScoped&&) = delete;

        DbConnectionPoolPtr m_pDbConnectionPool;
        QSqlDatabase m_dbConnection;
    };

  private:
    DbConnectionPool(const DbConnectionPool&) = delete;
    DbConnectionPool(const DbConnectionPool&&) = delete;

    friend class ThreadLocalScoped;
    bool createThreadLocalConnection();
    void destroyThreadLocalConnection();

    const DbConnection m_prototypeConnection;

    QAtomicInt m_connectionCounter;

    QThreadStorage<DbConnection*> m_threadLocalConnections;

};

} // namespace mixxx


#endif // MIXXX_DBCONNECTIONPOOL_H
