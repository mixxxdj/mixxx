#include "library/searchqueriesstorage.h"

#include <QHash>
#include <QList>
#include <QRegularExpression>
#include <QSet>
#include <QVariantList>
#include <algorithm>

#include "library/searchqueryparser.h"
#include "preferences/configobject.h"
#include "track/keyutils.h"
#include "util/assert.h"

namespace mixxx {

namespace {

const QString kSavedQueriesConfigGroup = QStringLiteral("[SearchQueries]");

const QHash<QString, QPair<const char*, const char*>> kChipFields = {
        {"artist", {"Artist", "artist"}},
        {"a", {"Artist", "artist"}},
        {"album", {"Album", "album"}},
        {"al", {"Album", "album"}},
        {"title", {"Title", "title"}},
        {"t", {"Title", "title"}},
        {"genre", {"Genre", "genre"}},
        {"g", {"Genre", "genre"}},
        {"composer", {"Composer", "composer"}},
        {"cp", {"Composer", "composer"}},
        {"comment", {"Comment", "comment"}},
        {"cm", {"Comment", "comment"}},
        {"bpm", {"BPM", "bpm"}},
        {"b", {"BPM", "bpm"}},
        {"key", {"Key", "key"}},
        {"k", {"Key", "key"}},
        {"year", {"Year", "year"}},
        {"y", {"Year", "year"}},
};

} // anonymous namespace

QStringList SearchQueriesStorage::loadQueries(const UserSettingsPointer& pConfig) {
    VERIFY_OR_DEBUG_ASSERT(pConfig) {
        return {};
    }

    QList<ConfigKey> queryKeys = pConfig->getKeysWithGroup(kSavedQueriesConfigGroup);
    // QMap orders the keys lexicographically, so sort by the numeric item
    // keys to restore the queries in the order they have been saved
    // (newest first).
    std::stable_sort(queryKeys.begin(),
            queryKeys.end(),
            [](const ConfigKey& lhs, const ConfigKey& rhs) {
                return lhs.item.toInt() < rhs.item.toInt();
            });

    QStringList queries;
    queries.reserve(queryKeys.size());
    QSet<QString> seenQueries;
    for (const auto& queryKey : queryKeys) {
        const QString queryString = pConfig->getValueString(queryKey).trimmed();
        if (queryString.isEmpty() || seenQueries.contains(queryString)) {
            continue;
        }
        queries.append(queryString);
        seenQueries.insert(queryString);
    }
    return queries;
}

void SearchQueriesStorage::saveQueries(
        const UserSettingsPointer& pConfig,
        const QStringList& queries) {
    VERIFY_OR_DEBUG_ASSERT(pConfig) {
        return;
    }

    const QList<ConfigKey> queryKeys = pConfig->getKeysWithGroup(kSavedQueriesConfigGroup);
    for (const auto& queryKey : queryKeys) {
        pConfig->remove(queryKey);
    }

    const int numQueries = std::min(
            queries.size(), static_cast<qsizetype>(kMaxQueries));
    for (int index = 0; index < numQueries; ++index) {
        const QString queryString = queries.at(index).trimmed();
        if (queryString.isEmpty()) {
            continue;
        }
        pConfig->setValue(
                ConfigKey(kSavedQueriesConfigGroup, QString::number(index)),
                queryString);
    }
}

QVariantMap SearchQueriesStorage::parseQuery(const QString& query) {
    QVariantMap result;
    QVariantList tokens;
    QStringList freeTextParts;

    const QStringList words = SearchQueryParser::splitQueryIntoWords(query);
    for (const QString& word : words) {
        QVariantMap token;
        bool isChip = false;
        if (!word.startsWith('-') && !word.startsWith('~')) {
            if (word.startsWith(QStringLiteral("key_id:"))) {
                bool ok = false;
                const int keyId = word.mid(7).toInt(&ok);
                const auto key = KeyUtils::keyFromNumericValue(keyId);
                if (ok && key != mixxx::track::io::key::INVALID) {
                    token.insert(QStringLiteral("name"), QStringLiteral("Key"));
                    token.insert(QStringLiteral("query"), QStringLiteral("key"));
                    token.insert(QStringLiteral("value"),
                            KeyUtils::keyToString(
                                    key, KeyUtils::KeyNotation::OpenKey));
                    token.insert(QStringLiteral("keyId"), keyId);
                    tokens.append(token);
                    isChip = true;
                }
            } else {
                static const QRegularExpression fieldMatcher(
                        QStringLiteral("^([a-zA-Z]+):(.*)$"));
                const auto match = fieldMatcher.match(word);
                if (match.hasMatch()) {
                    const auto fieldEntry =
                            kChipFields.constFind(match.captured(1).toLower());
                    QString value = match.captured(2);
                    if (fieldEntry != kChipFields.constEnd() && !value.isEmpty()) {
                        bool exact = false;
                        if (value.startsWith('=')) {
                            exact = true;
                            value = value.mid(1);
                        }
                        if (value.length() >= 2 && value.startsWith('"') &&
                                value.endsWith('"')) {
                            value = value.mid(1, value.length() - 2);
                        }
                        if (!value.isEmpty()) {
                            if (exact) {
                                value.prepend('=');
                            }
                            token.insert(QStringLiteral("name"),
                                    QString::fromUtf8(fieldEntry->first));
                            token.insert(QStringLiteral("query"),
                                    QString::fromUtf8(fieldEntry->second));
                            token.insert(QStringLiteral("value"), value);
                            token.insert(QStringLiteral("keyId"), 0);
                            tokens.append(token);
                            isChip = true;
                        }
                    }
                }
            }
        }
        if (!isChip) {
            freeTextParts.append(word);
        }
    }

    result.insert(QStringLiteral("tokens"), tokens);
    result.insert(QStringLiteral("freeText"), freeTextParts.join(' '));
    return result;
}

} // namespace mixxx
