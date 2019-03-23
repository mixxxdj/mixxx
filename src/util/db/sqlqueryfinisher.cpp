#include "util/db/sqlqueryfinisher.h"

#include "util/assert.h"


bool SqlQueryFinisher::finish() {
    if (m_query.isActive()) {
        m_query.finish();
        release();
        return true;
    } else {
        return false;
    }
}

void SqlQueryFinisher::release() {
    m_query = QSqlQuery();
    DEBUG_ASSERT(!m_query.isActive());
}
