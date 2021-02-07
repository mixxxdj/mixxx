#pragma once

#include <QSqlDatabase>

#include "util/assert.h"

class DAO {
  public:
    virtual ~DAO() = default;

    virtual void initialize(const QSqlDatabase& database) {
        DEBUG_ASSERT(!m_database.isOpen());
        m_database = database;
    }

    const QSqlDatabase& database() const {
        return m_database;
    }

  protected:
    QSqlDatabase m_database;
};
