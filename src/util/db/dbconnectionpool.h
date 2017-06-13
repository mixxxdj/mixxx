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
    static DbConnectionPoolPtr create(
            const DbConnection::Params& params,
            const QString& connectionName) {
        return std::make_shared<DbConnectionPool>(params, connectionName);
    }

    DbConnectionPool(
            const DbConnection::Params& params,
            const QString& connectionName);

    // Returns a database connection for the current thread, that has
    // previously been created (see above). The returned connection
    // is only valid within the current thread!
    QSqlDatabase threadLocalConnection() const;

    // Temporarily creates and destroys a thread-local database connection
    class ThreadLocalScope final {
      public:
        explicit ThreadLocalScope(
                DbConnectionPoolPtr pDbConnectionPool = DbConnectionPoolPtr());
        ThreadLocalScope(
                ThreadLocalScope&& other) = default;
        ~ThreadLocalScope();

        operator bool() const {
            return m_pDbConnectionPool != nullptr;
        }

        operator QSqlDatabase() const {
            return m_dbConnection;
        }

      private:
        ThreadLocalScope(const ThreadLocalScope&) = delete;
        ThreadLocalScope& operator=(const ThreadLocalScope&) = delete;
        ThreadLocalScope& operator=(ThreadLocalScope&&) = delete;

        DbConnectionPoolPtr m_pDbConnectionPool;
        QSqlDatabase m_dbConnection;
    };

  private:
    DbConnectionPool(const DbConnectionPool&) = delete;
    DbConnectionPool(const DbConnectionPool&&) = delete;

    friend class ThreadLocalScope;
    bool createThreadLocalConnection();
    void destroyThreadLocalConnection();

    const DbConnection m_prototypeConnection;

    QAtomicInt m_connectionCounter;

    QThreadStorage<DbConnection*> m_threadLocalConnections;

};

} // namespace mixxx


#endif // MIXXX_DBCONNECTIONPOOL_H
