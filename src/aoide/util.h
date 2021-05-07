#pragma once

#include <QMetaType>
#include <QUrlQuery>

namespace aoide {

struct Pagination {
    quint64 offset = 0;
    quint64 limit = 0;

    void addToQuery(QUrlQuery* query) const;
};

} // namespace aoide

Q_DECLARE_METATYPE(aoide::Pagination);
