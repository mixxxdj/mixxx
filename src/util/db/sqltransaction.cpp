#include "util/db/sqltransaction.h"

#include "util/logger.h"
#include "util/assert.h"


namespace {

const mixxx::Logger kLogger("SqlTransaction");

inline
bool beginTransaction(QSqlDatabase database) {
    if (!database.isOpen()) {
        // Should only happen during tests
        kLogger.warning()
                << "Failed to begin SQL database transaction on"
                << database.connectionName();
        return false;
    }
    if (database.transaction()) {
        kLogger.debug()
                << "Started new SQL database transaction on"
                << database.connectionName();
        return true;
    } else {
        kLogger.warning()
                << "Failed to begin SQL database transaction on"
                << database.connectionName();
        return false;
    }
}

} // anonymous namespace

SqlTransaction::SqlTransaction(
        const QSqlDatabase& database)
    : m_database(database), // implicitly shared (not copied)
      m_active(beginTransaction(m_database)) {
}

SqlTransaction::SqlTransaction(
        SqlTransaction&& other)
    : m_database(std::move(other.m_database)), // implicitly shared (not moved)
      m_active(other.m_active) {
    other.release();
}

SqlTransaction::~SqlTransaction() {
    if (m_active && m_database.isOpen()) {
        rollback();
    }
}

void SqlTransaction::release() {
    m_active = false;
}

bool SqlTransaction::commit() {
    DEBUG_ASSERT(m_active);
    if (!m_database.isOpen()) {
        kLogger.warning()
                << "Failed to commit transaction: No open SQL database connection";
        return false;
    }
    if (m_database.commit()) {
        kLogger.debug()
                << "Committed SQL database transaction on"
                << m_database.connectionName();
        release(); // commit/rollback only once
        return true;
    } else {
        kLogger.warning()
                << "Failed to commit SQL database transaction on"
                 << m_database.connectionName();
        return false;
    }
}

bool SqlTransaction::rollback() {
    DEBUG_ASSERT(m_active);
    if (!m_database.isOpen()) {
        kLogger.warning()
                << "Failed to rollback transaction: No open SQL database connection";
        return false;
    }
    if (m_database.rollback()) {
        kLogger.debug()
                << "Rolled back SQL database transaction on"
                << m_database.connectionName();
        release(); // commit/rollback only once
        return true;
    } else {
        kLogger.warning()
                << "Failed to rollback SQL database transaction on"
                << m_database.connectionName();
        return false;
    }
}
