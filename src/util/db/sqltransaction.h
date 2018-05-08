#ifndef MIXXX_SQLTRANSACTION_H
#define MIXXX_SQLTRANSACTION_H


#include <QSqlDatabase>


class SqlTransaction final {
  public:
    explicit SqlTransaction(QSqlDatabase database);
    SqlTransaction(SqlTransaction&& other);
    ~SqlTransaction();

    QSqlDatabase& database() {
        return m_database;
    }

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


#endif // MIXXX_SQLTRANSACTION_H
