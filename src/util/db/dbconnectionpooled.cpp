#include "util/db/dbconnectionpooled.h"

#include "util/logger.h"


namespace mixxx {

namespace {

const Logger kLogger("DbConnectionPooled");

} // anonymous namespace

DbConnectionPooled::operator QSqlDatabase() const {
    VERIFY_OR_DEBUG_ASSERT(m_pDbConnectionPool) {
        kLogger.critical()
                << "No connection pool";
        return QSqlDatabase(); // abort
    }
    const DbConnection* pDbConnection = m_pDbConnectionPool->threadLocalConnection();
    // The return pointer is at least valid until leaving this
    // function, because only the current thread is able to
    // remove this connection from the pool.
    VERIFY_OR_DEBUG_ASSERT(pDbConnection) {
        kLogger.critical()
                << "Thread-local database connection not found";
        return QSqlDatabase(); // abort
    }
    kLogger.debug()
            << "Found thread-local database connection"
            << *pDbConnection;;
    return *pDbConnection;
}

} // namespace mixxx
