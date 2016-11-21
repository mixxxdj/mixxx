#include "util/db/sqlselectiterator.h"


SqlSelectIterator::SqlSelectIterator()
    : m_queryFinisher(m_query) {
    DEBUG_ASSERT(!m_query.isActive());
}

SqlSelectIterator::SqlSelectIterator(FwdSqlQuery query)
    : m_query(query),
      m_queryFinisher(m_query) {
    DEBUG_ASSERT(m_query.isActive());
    DEBUG_ASSERT(m_query.isSelect());
    // Verify that the query has just been executed and that
    // iteration over the result set has not begun yet.
    DEBUG_ASSERT(!m_query.isValid());
    // Verify that we are iterating over a forward-only result set.
    // This is not a requirement or precondition, but just a check
    // that the query has been executed with maximum efficiency.
    DEBUG_ASSERT(m_query.isForwardOnly());
}

SqlSelectIterator::SqlSelectIterator(SqlSelectIterator&& other)
    : m_query(other.m_query),
      m_queryFinisher(m_query) {
    other.release();
}

SqlSelectIterator::~SqlSelectIterator() {
}

void SqlSelectIterator::release() {
    m_queryFinisher.release();
    m_query = FwdSqlQuery();
}
