#include "library/searchqueries.h"

#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

#include "test/mixxxtest.h"
#include "track/keyutils.h"

namespace {

class SearchQueriesTest : public MixxxTest {};

TEST_F(SearchQueriesTest, SaveLoadRoundTrip) {
    QStringList queries = {
            QStringLiteral("artist:foo"),
            QStringLiteral("bpm:115-128"),
            QStringLiteral("artist:\"A Super Artist\" bpm:100"),
    };
    mixxx::SearchQueries::saveQueries(config(), queries);
    EXPECT_EQ(mixxx::SearchQueries::loadQueries(config()), queries);

    saveAndReloadConfig();
    EXPECT_EQ(mixxx::SearchQueries::loadQueries(config()), queries);
}

TEST_F(SearchQueriesTest, LoadSortedByNumericKey) {
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
    EXPECT_EQ(mixxx::SearchQueries::loadQueries(config()), expected);
}

TEST_F(SearchQueriesTest, LoadDeduplicatesAndSkipsEmpty) {
    config()->setValue(ConfigKey("[SearchQueries]", "0"), QString("artist:foo"));
    config()->setValue(ConfigKey("[SearchQueries]", "1"), QString(" "));
    config()->setValue(ConfigKey("[SearchQueries]", "2"), QString("artist:foo"));
    config()->setValue(ConfigKey("[SearchQueries]", "3"), QString("bpm:120"));

    QStringList expected = {
            QStringLiteral("artist:foo"),
            QStringLiteral("bpm:120"),
    };
    EXPECT_EQ(mixxx::SearchQueries::loadQueries(config()), expected);
}

TEST_F(SearchQueriesTest, SaveOverwritesPreviousEntries) {
    mixxx::SearchQueries::saveQueries(
            config(), {QStringLiteral("artist:foo"), QStringLiteral("bpm:120")});
    mixxx::SearchQueries::saveQueries(config(), {QStringLiteral("title:bar")});

    QStringList expected = {QStringLiteral("title:bar")};
    EXPECT_EQ(mixxx::SearchQueries::loadQueries(config()), expected);
}

TEST_F(SearchQueriesTest, SaveCapsListSize) {
    QStringList queries;
    for (int i = 0; i < mixxx::SearchQueries::kMaxQueries + 10; ++i) {
        queries.append(QString("query%1").arg(i));
    }
    mixxx::SearchQueries::saveQueries(config(), queries);

    QStringList loaded = mixxx::SearchQueries::loadQueries(config());
    EXPECT_EQ(loaded.size(), mixxx::SearchQueries::kMaxQueries);
    // The oldest entries beyond the cap have been dropped.
    EXPECT_EQ(loaded.first(), queries.first());
    EXPECT_EQ(loaded.last(), queries.at(mixxx::SearchQueries::kMaxQueries - 1));
}

TEST_F(SearchQueriesTest, SaveSkipsEmptyQueries) {
    mixxx::SearchQueries::saveQueries(
            config(),
            {QString("artist:foo"), QString("  "), QString("bpm:120"), QString()});

    QStringList expected = {
            QStringLiteral("artist:foo"),
            QStringLiteral("bpm:120"),
    };
    EXPECT_EQ(mixxx::SearchQueries::loadQueries(config()), expected);
}

namespace {

QString entryToQueryString(const QVariantMap& parsed) {
    return mixxx::SearchQueries::serializeQuery(
            parsed.value(QStringLiteral("tokens")).toList(),
            parsed.value(QStringLiteral("freeText")).toString());
}

} // namespace

TEST_F(SearchQueriesTest, SerializeToken) {
    QVariantMap keyToken = {
            {QStringLiteral("query"), QStringLiteral("key")},
            {QStringLiteral("value"), QStringLiteral("11d")},
            {QStringLiteral("keyId"), 11},
    };
    EXPECT_QSTRING_EQ("key_id:11", mixxx::SearchQueries::serializeToken(keyToken));

    QVariantMap exactToken = {
            {QStringLiteral("query"), QStringLiteral("artist")},
            {QStringLiteral("value"), QStringLiteral("=A Super Artist")},
            {QStringLiteral("keyId"), 0},
    };
    EXPECT_QSTRING_EQ("artist:=\"A Super Artist\"",
            mixxx::SearchQueries::serializeToken(exactToken));

    // Any whitespace character (not just ' ') triggers quoting.
    QVariantMap tabToken = {
            {QStringLiteral("query"), QStringLiteral("artist")},
            {QStringLiteral("value"), QStringLiteral("a\tb")},
            {QStringLiteral("keyId"), 0},
    };
    EXPECT_QSTRING_EQ("artist:\"a\tb\"",
            mixxx::SearchQueries::serializeToken(tabToken));

    QVariantMap artistToken = {
            {QStringLiteral("query"), QStringLiteral("artist")},
            {QStringLiteral("value"), QStringLiteral("foo")},
            {QStringLiteral("keyId"), 0},
    };
    QVariantMap bpmToken = {
            {QStringLiteral("query"), QStringLiteral("bpm")},
            {QStringLiteral("value"), QStringLiteral("120")},
            {QStringLiteral("keyId"), 0},
    };
    EXPECT_QSTRING_EQ("artist:foo bpm:120 hello world",
            mixxx::SearchQueries::serializeQuery(
                    {artistToken, bpmToken}, QStringLiteral("hello world")));
}

TEST_F(SearchQueriesTest, ParseQueryChips) {
    const QVariantMap parsed = mixxx::SearchQueries::parseQuery(
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
    // The display value follows the configured key notation;
    // serialization keeps the keyId.
    EXPECT_QSTRING_EQ(
            KeyUtils::keyToString(KeyUtils::keyFromNumericValue(11)),
            key.value(QStringLiteral("value")).toString());

    EXPECT_TRUE(parsed.value(QStringLiteral("freeText")).toString().isEmpty());

    EXPECT_EQ(entryToQueryString(parsed),
            QStringLiteral("artist:foo bpm:=115-128 key_id:11"));
}

TEST_F(SearchQueriesTest, ParseQueryQuotedValues) {
    const QVariantMap parsed = mixxx::SearchQueries::parseQuery(
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

TEST_F(SearchQueriesTest, ParseQueryExactQuotedValue) {
    const QVariantMap parsed = mixxx::SearchQueries::parseQuery(
            QStringLiteral("artist:=\"A Super Artist\""));
    const QVariantList tokens = parsed.value(QStringLiteral("tokens")).toList();
    ASSERT_EQ(tokens.size(), 1);
    EXPECT_QSTRING_EQ("=A Super Artist",
            tokens.at(0).toMap().value(QStringLiteral("value")).toString());
    EXPECT_EQ(entryToQueryString(parsed),
            QStringLiteral("artist:=\"A Super Artist\""));
}

TEST_F(SearchQueriesTest, ParseQueryMultiExactQuotedChips) {
    // Pastes of full queries like 'artist:="Daft Punk" album:="Alive 2007"'
    // must become one exact-match chip per field:value word.
    const QVariantMap parsed = mixxx::SearchQueries::parseQuery(
            QStringLiteral("artist:=\"Daft Punk\" album:=\"Alive 2007\""));
    const QVariantList tokens = parsed.value(QStringLiteral("tokens")).toList();
    ASSERT_EQ(tokens.size(), 2);

    const QVariantMap artist = tokens.at(0).toMap();
    EXPECT_QSTRING_EQ("Artist", artist.value(QStringLiteral("name")).toString());
    EXPECT_QSTRING_EQ("artist", artist.value(QStringLiteral("query")).toString());
    EXPECT_QSTRING_EQ("=Daft Punk",
            artist.value(QStringLiteral("value")).toString());
    EXPECT_EQ(artist.value(QStringLiteral("keyId")).toInt(), 0);

    const QVariantMap album = tokens.at(1).toMap();
    EXPECT_QSTRING_EQ("Album", album.value(QStringLiteral("name")).toString());
    EXPECT_QSTRING_EQ("=Alive 2007",
            album.value(QStringLiteral("value")).toString());

    EXPECT_TRUE(parsed.value(QStringLiteral("freeText")).toString().isEmpty());
    EXPECT_EQ(entryToQueryString(parsed),
            QStringLiteral("artist:=\"Daft Punk\" album:=\"Alive 2007\""));
}

TEST_F(SearchQueriesTest, ParseQueryChipWithLeftoverFreeText) {
    const QVariantMap parsed = mixxx::SearchQueries::parseQuery(
            QStringLiteral("artist:\"Daft Punk\" 2007"));
    const QVariantList tokens = parsed.value(QStringLiteral("tokens")).toList();
    ASSERT_EQ(tokens.size(), 1);
    EXPECT_QSTRING_EQ("Artist",
            tokens.at(0).toMap().value(QStringLiteral("name")).toString());
    EXPECT_QSTRING_EQ("Daft Punk",
            tokens.at(0).toMap().value(QStringLiteral("value")).toString());
    EXPECT_QSTRING_EQ("2007",
            parsed.value(QStringLiteral("freeText")).toString());
    EXPECT_EQ(entryToQueryString(parsed),
            QStringLiteral("artist:\"Daft Punk\" 2007"));
}

TEST_F(SearchQueriesTest, ParseQueryMultiMixedChips) {
    const QVariantMap parsed = mixxx::SearchQueries::parseQuery(
            QStringLiteral("bpm:127-129 album:X"));
    const QVariantList tokens = parsed.value(QStringLiteral("tokens")).toList();
    ASSERT_EQ(tokens.size(), 2);
    EXPECT_QSTRING_EQ("BPM",
            tokens.at(0).toMap().value(QStringLiteral("name")).toString());
    EXPECT_QSTRING_EQ("127-129",
            tokens.at(0).toMap().value(QStringLiteral("value")).toString());
    EXPECT_QSTRING_EQ("Album",
            tokens.at(1).toMap().value(QStringLiteral("name")).toString());
    EXPECT_QSTRING_EQ("X",
            tokens.at(1).toMap().value(QStringLiteral("value")).toString());
    EXPECT_EQ(entryToQueryString(parsed),
            QStringLiteral("bpm:127-129 album:X"));
}

TEST_F(SearchQueriesTest, ParseQueryUnclosedQuoteStaysFreeText) {
    // An unclosed quoted argument can never round-trip as a chip value, so
    // the word stays free text instead of forming a chip with a stray quote.
    const QVariantMap parsed = mixxx::SearchQueries::parseQuery(
            QStringLiteral("artist:\"Daft"));
    const QVariantList tokens = parsed.value(QStringLiteral("tokens")).toList();
    ASSERT_EQ(tokens.size(), 0);
    EXPECT_QSTRING_EQ("artist:\"Daft",
            parsed.value(QStringLiteral("freeText")).toString());
}

TEST_F(SearchQueriesTest, ParseQueryDoubleEqualsMarker) {
    const QVariantMap parsed = mixxx::SearchQueries::parseQuery(
            QStringLiteral("comment:=="));
    const QVariantList tokens = parsed.value(QStringLiteral("tokens")).toList();
    ASSERT_EQ(tokens.size(), 1);
    EXPECT_QSTRING_EQ("==",
            tokens.at(0).toMap().value(QStringLiteral("value")).toString());
    EXPECT_EQ(entryToQueryString(parsed), QStringLiteral("comment:=="));
}

TEST_F(SearchQueriesTest, ParseQueryNonChipsRemainFreeText) {
    const QVariantMap parsed = mixxx::SearchQueries::parseQuery(
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

TEST_F(SearchQueriesTest, ParseQueryEmptyValueStaysFreeText) {
    const QVariantMap parsed =
            mixxx::SearchQueries::parseQuery(QStringLiteral("artist: foo"));
    EXPECT_TRUE(parsed.value(QStringLiteral("tokens")).toList().isEmpty());
    EXPECT_QSTRING_EQ("artist: foo",
            parsed.value(QStringLiteral("freeText")).toString());
}

TEST_F(SearchQueriesTest, ParseQueryOrStaysVerbatimFreeText) {
    // Serialization moves the free text behind the chips, which would
    // turn an OR query into an AND query plus a literal "|" term. The
    // whole query must stay free text instead.
    const QString query = QStringLiteral("artist:a | title:b");
    const QVariantMap parsed = mixxx::SearchQueries::parseQuery(query);
    EXPECT_TRUE(parsed.value(QStringLiteral("tokens")).toList().isEmpty());
    EXPECT_QSTRING_EQ("artist:a | title:b",
            parsed.value(QStringLiteral("freeText")).toString());
    EXPECT_EQ(entryToQueryString(parsed), query);
}

TEST_F(SearchQueriesTest, ParseQueryQuoteInValueStaysFreeText) {
    // A value containing a quote itself cannot be serialized without
    // escaping, so the word stays free text. The other chip keeps its
    // place ahead of the free text (order loss is harmless for AND
    // queries) and the restored query parses into the same tokens again.
    const QVariantMap parsed = mixxx::SearchQueries::parseQuery(
            QStringLiteral("artist:foo\"bar title:X"));
    const QVariantList tokens = parsed.value(QStringLiteral("tokens")).toList();
    ASSERT_EQ(tokens.size(), 1);
    EXPECT_QSTRING_EQ("title", tokens.at(0).toMap().value(QStringLiteral("query")).toString());
    EXPECT_QSTRING_EQ("artist:foo\"bar",
            parsed.value(QStringLiteral("freeText")).toString());
    EXPECT_EQ(entryToQueryString(parsed),
            QStringLiteral("title:X artist:foo\"bar"));
    // Re-parsing the restored query serializes identically.
    const QVariantMap reparsed = mixxx::SearchQueries::parseQuery(
            QStringLiteral("title:X artist:foo\"bar"));
    EXPECT_EQ(entryToQueryString(reparsed),
            QStringLiteral("title:X artist:foo\"bar"));
}

} // namespace
