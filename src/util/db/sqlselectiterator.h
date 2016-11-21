#ifndef MIXXX_SQLSELECTITERATOR_H
#define MIXXX_SQLSELECTITERATOR_H


#include "util/db/fwdsqlquery.h"
#include "util/db/sqlqueryfinisher.h"

#include "util/assert.h"


// Iterate (unidirectional and only once) over the results of a query.
class SqlSelectIterator {
public:
    SqlSelectIterator(SqlSelectIterator&& other);
    virtual ~SqlSelectIterator();

    operator bool() const {
        return m_query.isActive();
    }

    bool next() {
        return m_query.next();
    }

    void release();

    // Disable copy construction and copy/move assignment
    SqlSelectIterator(const SqlSelectIterator&) = delete;
    SqlSelectIterator& operator=(const SqlSelectIterator&) = delete;
    SqlSelectIterator& operator=(SqlSelectIterator&&) = delete;

protected:
    SqlSelectIterator();
    explicit SqlSelectIterator(FwdSqlQuery query);

    const FwdSqlQuery& query() const {
        return m_query;
    }

private:
    FwdSqlQuery m_query; // implicitly shared
    SqlQueryFinisher m_queryFinisher;
};


#endif // MIXXX_SQLSELECTITERATOR_H
