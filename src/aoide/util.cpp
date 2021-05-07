#include "aoide/util.h"

#include "util/assert.h"

namespace aoide {

void Pagination::addToQuery(QUrlQuery* query) const {
    DEBUG_ASSERT(query);
    if (offset > 0) {
        query->addQueryItem("offset", QString::number(offset));
    }
    if (limit > 0) {
        query->addQueryItem("limit", QString::number(limit));
    }
}

} // namespace aoide
