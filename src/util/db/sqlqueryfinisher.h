#ifndef MIXXX_SQLQUERYFINISHER_H
#define MIXXX_SQLQUERYFINISHER_H


#include <QSqlQuery>


// Ensures that reusable queries are properly finished when
// leaving the corresponding execution scope. This will free
// resources of prepared statements until the next execution.
class SqlQueryFinisher final {
public:
    explicit SqlQueryFinisher(QSqlQuery query)
        : m_query(query) { // implicitly shared (not copied)
    }
    SqlQueryFinisher(SqlQueryFinisher&& other)
        : m_query(std::move(other.m_query)) { // implicitly shared (not moved)
        other.release();
    }
    ~SqlQueryFinisher() {
        finish();
    }

    bool finish();

    void release();

private:
    // Disable copy construction and copy/move assignment
    SqlQueryFinisher(const SqlQueryFinisher&) = delete;
    SqlQueryFinisher& operator=(const SqlQueryFinisher&) = delete;
    SqlQueryFinisher& operator=(SqlQueryFinisher&&) = delete;

    QSqlQuery m_query; // implicitly shared
};


#endif // MIXXX_SQLQUERYFINISHER_H
