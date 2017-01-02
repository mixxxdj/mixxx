#include "util/db/sqltransaction.h"

#include <QtDebug>

#include "util/assert.h"


namespace {
    inline
    bool beginTransaction(QSqlDatabase& database) {
        if (!database.isOpen()) {
            // Should only happen during tests
            qWarning() << "Failed to begin SQL database transaction on"
                    << database.connectionName();
            return false;
        }
        if (database.transaction()) {
            qDebug() << "Started new SQL database transaction on"
                << database.connectionName();
            return true;
        } else {
            qWarning() << "Failed to begin SQL database transaction on"
                << database.connectionName();
            return false;
        }
    }

} // anonymous namespace

SqlTransaction::SqlTransaction(QSqlDatabase database)
    : m_database(database), // implicitly shared (not copied)
      m_active(beginTransaction(m_database)) {
}

SqlTransaction::SqlTransaction(SqlTransaction&& other)
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
        qWarning() << "Failed to commit transaction: No open SQL database connection";
        return false;
    }
    if (m_database.commit()) {
        qDebug() << "Committed SQL database transaction on"
                << m_database.connectionName();
        release(); // commit/rollback only once
        return true;
    } else {
        qWarning() << "Failed to commit SQL database transaction on"
                 << m_database.connectionName();
        return false;
    }
}

bool SqlTransaction::rollback() {
    DEBUG_ASSERT(m_active);
    if (!m_database.isOpen()) {
        qWarning() << "Failed to rollback transaction: No open SQL database connection";
        return false;
    }
    if (m_database.rollback()) {
        qDebug() << "Rolled back SQL database transaction on"
                << m_database.connectionName();
        release(); // commit/rollback only once
        return true;
    } else {
        qWarning() << "Failed to rollback SQL database transaction on"
                << m_database.connectionName();
        return false;
    }
}
