#pragma once

#include <QSqlDatabase>


class SqlTransaction final {
  public:
    explicit SqlTransaction(
            const QSqlDatabase& database);
    SqlTransaction(SqlTransaction&& other);
    ~SqlTransaction();

    operator bool() const {
        return m_active;
    }

    bool commit();
    bool rollback();

    void release();

    // Disable copy construction and copy/move assignment
    SqlTransaction(const SqlTransaction&) = delete;
    SqlTransaction& operator=(const SqlTransaction&) = delete;
    SqlTransaction& operator=(SqlTransaction&&) = delete;

  private:
    QSqlDatabase m_database;
    bool m_active;
};
