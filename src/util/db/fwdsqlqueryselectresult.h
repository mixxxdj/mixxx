#pragma once

#include "util/db/fwdsqlquery.h"
#include "util/db/sqlqueryfinisher.h"

#include "util/assert.h"


// Visit the results of a forward-only query once.
//
// The constructor takes ownership of the wrapped query. While visiting
// the result set this query should only be accessed by the function
// query() from derived classes. The query is implicitly finished upon
// destruction, unless ownership on the query is reclaimed with release()
// beforehand.
//
// NOTE(uklotzde, 2017-01-06): Use move semantics although Qt currently
// uses implicit sharing behind the scenes. If Qt ever decides to implement
// move semantics this code will automatically benefit. The r-value reference
// to the query that is passed to the constructor documents the intention
// to take ownership. This is checked at compile time.
class FwdSqlQuerySelectResult {
public:
    FwdSqlQuerySelectResult(FwdSqlQuerySelectResult&& other);
    virtual ~FwdSqlQuerySelectResult() = default;

    bool next() {
        return m_query.next();
    }

    FwdSqlQuery release(); // release ownership

protected:
    FwdSqlQuerySelectResult();
    explicit FwdSqlQuerySelectResult(FwdSqlQuery&& query); // take ownership

    const FwdSqlQuery& query() const {
        return m_query;
    }

private:
    // Disable copy construction and copy/move assignment
    FwdSqlQuerySelectResult(const FwdSqlQuerySelectResult&) = delete;
    FwdSqlQuerySelectResult& operator=(const FwdSqlQuerySelectResult&) = delete;
    FwdSqlQuerySelectResult& operator=(FwdSqlQuerySelectResult&&) = delete;

    FwdSqlQuery m_query;
    SqlQueryFinisher m_queryFinisher;
};
