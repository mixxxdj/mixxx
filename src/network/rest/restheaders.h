#pragma once

#ifdef MIXXX_HAS_HTTP_SERVER

#include <QtGlobal>

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#include <QHttpHeaders>
#else
#include <QMultiMap>
#endif

namespace mixxx::network::rest {

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
using RestHeaders = QHttpHeaders;
#else
using RestHeaders = QMultiMap<QByteArray, QByteArray>;
#endif

} // namespace mixxx::network::rest

#endif // MIXXX_HAS_HTTP_SERVER
