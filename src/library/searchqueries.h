#pragma once

#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

#include "preferences/usersettings.h"

namespace mixxx {

class SearchQueries final {
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

    // Serializes a structured token (name, query, value, keyId) back into
    // its query-string form: key_id:N for key tokens, otherwise query:value
    // (quoted if the value contains whitespace, '='-prefixed for exact
    // matches).
    static QString serializeToken(const QVariantMap& token);

    // Joins serialized tokens and optional free text into a single query
    // string.
    static QString serializeQuery(
            const QVariantList& tokens, const QString& freeText);
};

} // namespace mixxx
