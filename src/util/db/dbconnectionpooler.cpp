#include "util/db/dbconnectionpooler.h"

#include "util/logger.h"


namespace mixxx {

namespace {

const Logger kLogger("DbConnectionPooler");

} // anonymous namespace

DbConnectionPooler::DbConnectionPooler(
        DbConnectionPoolPtr pDbConnectionPool) {
    if (pDbConnectionPool && pDbConnectionPool->createThreadLocalConnection()) {
        // m_pDbConnectionPool indicates if the thread-local connection has actually
        // been created during construction. Otherwise this instance is non-functional.
        m_pDbConnectionPool = std::move(pDbConnectionPool);
    }
}

DbConnectionPooler::~DbConnectionPooler() {
    if (m_pDbConnectionPool) {
        // Only destroy the thread-local connection if it has actually been created
        // during construction (see above).
        m_pDbConnectionPool->destroyThreadLocalConnection();
    }
}

DbConnectionPooler& DbConnectionPooler::operator=(DbConnectionPooler&& other) {
    m_pDbConnectionPool = std::move(other.m_pDbConnectionPool);
    // Move assignment should transfer ownership by invalidating
    // the other instance.
    DEBUG_ASSERT(!other);
}

} // namespace mixxx
