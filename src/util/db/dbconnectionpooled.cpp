#include "util/db/dbconnectionpooled.h"

#include "util/logger.h"


namespace mixxx {

namespace {

const Logger kLogger("DbConnectionPooled");

} // anonymous namespace

DbConnectionPooled::DbConnectionPooled(
        DbConnectionPoolPtr pDbConnectionPool) {
    if (pDbConnectionPool && pDbConnectionPool->createThreadLocalConnection()) {
        // m_pDbConnectionPool indicates if the thread-local connection has actually
        // been created during construction. Otherwise this instance is non-functional.
        m_pDbConnectionPool = std::move(pDbConnectionPool);
        m_sqlDatabase = m_pDbConnectionPool->threadLocalConnection();
    }
    VERIFY_OR_DEBUG_ASSERT(isOpen()) {
        kLogger.warning()
                << "Failed to create and open database connection";
    }
}

DbConnectionPooled::~DbConnectionPooled() {
    if (m_pDbConnectionPool) {
        VERIFY_OR_DEBUG_ASSERT(isOpen()) {
            kLogger.warning()
                    << "Database connection has been closed prematurely";
        }
        // Only destroy the thread-local connection if it has actually been created
        // during construction (see above).
        m_pDbConnectionPool->destroyThreadLocalConnection();
    }
}

QDebug operator<<(QDebug debug, const DbConnectionPooled& connection) {
    return debug
            << connection.name()
            << connection.m_sqlDatabase;
}

} // namespace mixxx
