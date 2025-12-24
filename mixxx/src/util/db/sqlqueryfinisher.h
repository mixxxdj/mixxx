#pragma once

#include <QSqlQuery>

#include "util/db/fwdsqlquery.h"

/// Ensures that reusable queries are properly finished when
/// leaving the corresponding execution scope. This will free
/// resources of prepared statements until the next execution.
class SqlQueryFinisher final {
  public:
    explicit SqlQueryFinisher(QSqlQuery* pQuery)
            : m_pQuery(pQuery) {
    }
    explicit SqlQueryFinisher(FwdSqlQuery* pQuery)
            : m_pQuery(pQuery) {
    }
    SqlQueryFinisher(SqlQueryFinisher&& other)
            : m_pQuery(other.m_pQuery) {
        other.m_pQuery = nullptr;
    }
    ~SqlQueryFinisher() {
        tryFinish();
    }

    bool tryFinish();

    void release() {
        m_pQuery = nullptr;
    }

  private:
    // Disable copy construction and copy/move assignment
    SqlQueryFinisher(const SqlQueryFinisher&) = delete;
    SqlQueryFinisher& operator=(const SqlQueryFinisher&) = delete;
    SqlQueryFinisher& operator=(SqlQueryFinisher&&) = delete;

    QSqlQuery* m_pQuery;
};
