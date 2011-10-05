#ifndef QUERYUTIL_H
#define QUERYUTIL_H

#include <QtDebug>

#define LOG_FAILED_QUERY(query) qDebug() << __FILE__ << __LINE__ << "FAILED QUERY [" \
    << query.executedQuery() << "]" << query.lastError()

#endif /* QUERYUTIL_H */
