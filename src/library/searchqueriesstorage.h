#pragma once

#include <QString>
#include <QStringList>
#include <QVariantMap>

#include "preferences/usersettings.h"

namespace mixxx {

class SearchQueriesStorage final {
  public:
    static constexpr int kMaxQueries = 50;

    static QStringList loadQueries(const UserSettingsPointer& pConfig);
    static void saveQueries(
            const UserSettingsPointer& pConfig,
            const QStringList& queries);

    // Splits a stored query string into structured search criteria tokens
    // (name, query, value, keyId) and the remaining free text. The value of
    // a token keeps a leading '=' marker for exact matches.
    static QVariantMap parseQuery(const QString& query);
};

} // namespace mixxx
