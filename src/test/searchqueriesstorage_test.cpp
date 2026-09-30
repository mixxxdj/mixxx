#include "library/searchqueriesstorage.h"

#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

#include "test/mixxxtest.h"

namespace {

class SearchQueriesStorageTest : public MixxxTest {};

TEST_F(SearchQueriesStorageTest, SaveLoadRoundTrip) {
    QStringList queries = {
            QStringLiteral("artist:foo"),
            QStringLiteral("bpm:115-128"),
            QStringLiteral("artist:\"A Super Artist\" bpm:100"),
    };
    mixxx::SearchQueriesStorage::saveQueries(config(), queries);
    EXPECT_EQ(mixxx::SearchQueriesStorage::loadQueries(config()), queries);

    saveAndReloadConfig();
    EXPECT_EQ(mixxx::SearchQueriesStorage::loadQueries(config()), queries);
}

TEST_F(SearchQueriesStorageTest, LoadSortedByNumericKey) {
    // QMap orders the keys lexicographically, so write the keys out of
    // numeric order to verify that the queries are restored chronologically
    // (newest first).
    config()->setValue(ConfigKey("[SearchQueries]", "2"), QString("query2"));
    config()->setValue(ConfigKey("[SearchQueries]", "10"), QString("query10"));
    config()->setValue(ConfigKey("[SearchQueries]", "1"), QString("query1"));
    config()->setValue(ConfigKey("[SearchQueries]", "0"), QString("query0"));

    QStringList expected = {
            QStringLiteral("query0"),
            QStringLiteral("query1"),
            QStringLiteral("query2"),
            QStringLiteral("query10"),
    };
    EXPECT_EQ(mixxx::SearchQueriesStorage::loadQueries(config()), expected);
}

TEST_F(SearchQueriesStorageTest, LoadDeduplicatesAndSkipsEmpty) {
    config()->setValue(ConfigKey("[SearchQueries]", "0"), QString("artist:foo"));
    config()->setValue(ConfigKey("[SearchQueries]", "1"), QString(" "));
    config()->setValue(ConfigKey("[SearchQueries]", "2"), QString("artist:foo"));
    config()->setValue(ConfigKey("[SearchQueries]", "3"), QString("bpm:120"));

    QStringList expected = {
            QStringLiteral("artist:foo"),
            QStringLiteral("bpm:120"),
    };
    EXPECT_EQ(mixxx::SearchQueriesStorage::loadQueries(config()), expected);
}

TEST_F(SearchQueriesStorageTest, SaveOverwritesPreviousEntries) {
    mixxx::SearchQueriesStorage::saveQueries(
            config(), {QStringLiteral("artist:foo"), QStringLiteral("bpm:120")});
    mixxx::SearchQueriesStorage::saveQueries(config(), {QStringLiteral("title:bar")});

    QStringList expected = {QStringLiteral("title:bar")};
    EXPECT_EQ(mixxx::SearchQueriesStorage::loadQueries(config()), expected);
}

TEST_F(SearchQueriesStorageTest, SaveCapsListSize) {
    QStringList queries;
    for (int i = 0; i < mixxx::SearchQueriesStorage::kMaxQueries + 10; ++i) {
        queries.append(QString("query%1").arg(i));
    }
    mixxx::SearchQueriesStorage::saveQueries(config(), queries);

    QStringList loaded = mixxx::SearchQueriesStorage::loadQueries(config());
    EXPECT_EQ(loaded.size(), mixxx::SearchQueriesStorage::kMaxQueries);
    // The oldest entries beyond the cap have been dropped.
    EXPECT_EQ(loaded.first(), queries.first());
    EXPECT_EQ(loaded.last(), queries.at(mixxx::SearchQueriesStorage::kMaxQueries - 1));
}

TEST_F(SearchQueriesStorageTest, SaveSkipsEmptyQueries) {
    mixxx::SearchQueriesStorage::saveQueries(
            config(),
            {QString("artist:foo"), QString("  "), QString("bpm:120"), QString()});

    QStringList expected = {
            QStringLiteral("artist:foo"),
            QStringLiteral("bpm:120"),
    };
    EXPECT_EQ(mixxx::SearchQueriesStorage::loadQueries(config()), expected);
}

namespace {

QString tokenToQueryString(const QVariantMap& token) {
    const int keyId = token.value(QStringLiteral("keyId")).toInt();
    if (keyId > 0) {
        return QStringLiteral("key_id:%1").arg(keyId);
    }
    QString value = token.value(QStringLiteral("value")).toString();
    const bool exact = value.startsWith('=');
    if (exact) {
        value = value.mid(1);
    }
    QString argument = value;
    if (argument.contains(' ')) {
        argument = QStringLiteral("\"%1\"").arg(argument);
    }
    if (exact) {
        argument = QStringLiteral("=") + argument;
    }
    return token.value(QStringLiteral("query")).toString() + ":" + argument;
}

QString entryToQueryString(const QVariantMap& parsed) {
    QStringList parts;
    const QVariantList tokens = parsed.value(QStringLiteral("tokens")).toList();
    parts.reserve(tokens.size());
    for (const auto& tokenValue : tokens) {
        parts.append(tokenToQueryString(tokenValue.toMap()));
    }
    const QString freeText = parsed.value(QStringLiteral("freeText")).toString();
    if (!freeText.isEmpty()) {
        parts.append(freeText);
    }
    return parts.join(' ');
}

} // namespace

TEST_F(SearchQueriesStorageTest, ParseQueryChips) {
    const QVariantMap parsed = mixxx::SearchQueriesStorage::parseQuery(
            QStringLiteral("artist:foo bpm:=115-128 key_id:11"));
    const QVariantList tokens = parsed.value(QStringLiteral("tokens")).toList();
    ASSERT_EQ(tokens.size(), 3);

    const QVariantMap artist = tokens.at(0).toMap();
    EXPECT_QSTRING_EQ("Artist", artist.value(QStringLiteral("name")).toString());
    EXPECT_QSTRING_EQ("artist", artist.value(QStringLiteral("query")).toString());
    EXPECT_QSTRING_EQ("foo", artist.value(QStringLiteral("value")).toString());
    EXPECT_EQ(artist.value(QStringLiteral("keyId")).toInt(), 0);

    const QVariantMap bpm = tokens.at(1).toMap();
    EXPECT_QSTRING_EQ("BPM", bpm.value(QStringLiteral("name")).toString());
    EXPECT_QSTRING_EQ("=115-128", bpm.value(QStringLiteral("value")).toString());

    const QVariantMap key = tokens.at(2).toMap();
    EXPECT_QSTRING_EQ("Key", key.value(QStringLiteral("name")).toString());
    EXPECT_EQ(key.value(QStringLiteral("keyId")).toInt(), 11);
    EXPECT_QSTRING_EQ("11d", key.value(QStringLiteral("value")).toString());

    EXPECT_TRUE(parsed.value(QStringLiteral("freeText")).toString().isEmpty());

    EXPECT_EQ(entryToQueryString(parsed),
            QStringLiteral("artist:foo bpm:=115-128 key_id:11"));
}

TEST_F(SearchQueriesStorageTest, ParseQueryQuotedValues) {
    const QVariantMap parsed = mixxx::SearchQueriesStorage::parseQuery(
            QStringLiteral("artist:\"A Super Artist\" bpm:100"));
    const QVariantList tokens = parsed.value(QStringLiteral("tokens")).toList();
    ASSERT_EQ(tokens.size(), 2);
    EXPECT_QSTRING_EQ("A Super Artist",
            tokens.at(0).toMap().value(QStringLiteral("value")).toString());
    EXPECT_QSTRING_EQ("100",
            tokens.at(1).toMap().value(QStringLiteral("value")).toString());
    EXPECT_EQ(entryToQueryString(parsed),
            QStringLiteral("artist:\"A Super Artist\" bpm:100"));
}

TEST_F(SearchQueriesStorageTest, ParseQueryExactQuotedValue) {
    const QVariantMap parsed = mixxx::SearchQueriesStorage::parseQuery(
            QStringLiteral("artist:=\"A Super Artist\""));
    const QVariantList tokens = parsed.value(QStringLiteral("tokens")).toList();
    ASSERT_EQ(tokens.size(), 1);
    EXPECT_QSTRING_EQ("=A Super Artist",
            tokens.at(0).toMap().value(QStringLiteral("value")).toString());
    EXPECT_EQ(entryToQueryString(parsed),
            QStringLiteral("artist:=\"A Super Artist\""));
}

TEST_F(SearchQueriesStorageTest, ParseQueryDoubleEqualsMarker) {
    const QVariantMap parsed = mixxx::SearchQueriesStorage::parseQuery(
            QStringLiteral("comment:=="));
    const QVariantList tokens = parsed.value(QStringLiteral("tokens")).toList();
    ASSERT_EQ(tokens.size(), 1);
    EXPECT_QSTRING_EQ("==",
            tokens.at(0).toMap().value(QStringLiteral("value")).toString());
    EXPECT_EQ(entryToQueryString(parsed), QStringLiteral("comment:=="));
}

TEST_F(SearchQueriesStorageTest, ParseQueryNonChipsRemainFreeText) {
    const QVariantMap parsed = mixxx::SearchQueriesStorage::parseQuery(
            QStringLiteral(
                    "a:foo t:bar -year:1990 ~key:8d track:3 foo:bar hello world"));
    const QVariantList tokens = parsed.value(QStringLiteral("tokens")).toList();
    ASSERT_EQ(tokens.size(), 2);
    EXPECT_QSTRING_EQ("Artist",
            tokens.at(0).toMap().value(QStringLiteral("name")).toString());
    EXPECT_QSTRING_EQ("Title",
            tokens.at(1).toMap().value(QStringLiteral("name")).toString());
    EXPECT_QSTRING_EQ("-year:1990 ~key:8d track:3 foo:bar hello world",
            parsed.value(QStringLiteral("freeText")).toString());
    EXPECT_EQ(entryToQueryString(parsed),
            QStringLiteral(
                    "artist:foo title:bar -year:1990 ~key:8d track:3 foo:bar hello world"));
}

TEST_F(SearchQueriesStorageTest, ParseQueryEmptyValueStaysFreeText) {
    const QVariantMap parsed =
            mixxx::SearchQueriesStorage::parseQuery(QStringLiteral("artist: foo"));
    EXPECT_TRUE(parsed.value(QStringLiteral("tokens")).toList().isEmpty());
    EXPECT_QSTRING_EQ("artist: foo",
            parsed.value(QStringLiteral("freeText")).toString());
}

} // namespace
