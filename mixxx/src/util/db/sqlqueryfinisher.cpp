#include "util/db/sqlqueryfinisher.h"

bool SqlQueryFinisher::tryFinish() {
    if (!m_pQuery || !m_pQuery->isActive()) {
        return false;
    }
    m_pQuery->finish();
    m_pQuery = nullptr;
    return true;
}
