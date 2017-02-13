#include "util/db/fwdsqlqueryselectresult.h"


FwdSqlQuerySelectResult::FwdSqlQuerySelectResult()
    : m_queryFinisher(m_query) {
    DEBUG_ASSERT(!m_query.isActive());
}

// NOTE(uklotzde): The query is passed as an r-value reference to
// indicate that ownership is transfered. Qt uses implicit sharing
// instead of actually moving the contents of the object. This might
// be less efficient than actually moving the object's contents, but
// meets all requirements.
FwdSqlQuerySelectResult::FwdSqlQuerySelectResult(FwdSqlQuery&& query)
    : m_query(std::move(query)),
      // Pass a reference to the member to m_queryFinisher, because
      // the contents of the r-value reference parameter might have
      // been moved!
      m_queryFinisher(m_query) {
    DEBUG_ASSERT(m_query.isActive());
    DEBUG_ASSERT(m_query.isSelect());
    // Verify that the query has just been executed and that
    // iteration over the result set has not yet begun.
    DEBUG_ASSERT(!m_query.isValid());
    // Verify that we are visiting a forward-only result set. This is
    // not a requirement or precondition, but just a checks that the
    // query has been executed with maximum efficiency.
    DEBUG_ASSERT(m_query.isForwardOnly());
}

FwdSqlQuerySelectResult::FwdSqlQuerySelectResult(FwdSqlQuerySelectResult&& other)
    : m_query(std::move(other.m_query)),
      m_queryFinisher(std::move(other.m_queryFinisher)) {
}

FwdSqlQuery FwdSqlQuerySelectResult::release() {
    FwdSqlQuery query(std::move(m_query));
    // Ensure that the wrapped query is inaccessible after moving it!
    m_query = FwdSqlQuery();
    m_queryFinisher.release();
    return query;
}
