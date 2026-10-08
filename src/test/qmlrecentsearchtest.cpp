#include <gtest/gtest.h>

#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

#include "library/searchqueriesstorage.h"
#include "qml/qmlrecentsearchmodel.h"
#include "test/mixxxtest.h"

namespace {

// Tests that QmlRecentSearchModel owns the whole load/parse/serialize/save
// cycle for recent library searches, without any QML-side processing. The
// query string parsing and serialization itself are covered by
// SearchQueriesStorageTest.
class QmlRecentSearchTest : public MixxxTest {};

QVariantMap token(const QString& name, const QString& query, const QString& value, int keyId) {
    return {
            {QStringLiteral("name"), name},
            {QStringLiteral("query"), query},
            {QStringLiteral("value"), value},
            {QStringLiteral("keyId"), keyId},
    };
}

TEST_F(QmlRecentSearchTest, LoadBuildsRowsFromConfig) {
    mixxx::SearchQueriesStorage::saveQueries(config(),
            {QStringLiteral("artist:\"A Super Artist\" bpm:120"),
                    QStringLiteral("hello world")});

    mixxx::qml::QmlRecentSearchModel model(config());
    EXPECT_EQ(model.rowCount(), 2);

    const QVariantMap entry = model.get(0);
    EXPECT_QSTRING_EQ("artist:\"A Super Artist\" bpm:120",
            entry.value(QStringLiteral("queryString")).toString());
    const QVariantList tokens = entry.value(QStringLiteral("tokens")).toList();
    ASSERT_EQ(tokens.size(), 2);
    EXPECT_QSTRING_EQ("Artist", tokens.at(0).toMap().value(QStringLiteral("name")).toString());
    EXPECT_QSTRING_EQ("A Super Artist",
            tokens.at(0).toMap().value(QStringLiteral("value")).toString());
    EXPECT_QSTRING_EQ("artist",
            tokens.at(0).toMap().value(QStringLiteral("query")).toString());
    EXPECT_QSTRING_EQ("BPM", tokens.at(1).toMap().value(QStringLiteral("name")).toString());

    const QVariantMap helloWorldEntry = model.get(1);
    EXPECT_QSTRING_EQ("hello world",
            helloWorldEntry.value(QStringLiteral("freeText")).toString());

    EXPECT_TRUE(model.get(99).isEmpty());
}

TEST_F(QmlRecentSearchTest, PersistInsertsAtFrontAndSavesConfig) {
    mixxx::qml::QmlRecentSearchModel model(config());

    const int row = model.persist({token("Artist", "artist", "foo", 0)},
            QStringLiteral("hello world"),
            -1);
    EXPECT_EQ(row, 0);
    EXPECT_EQ(model.rowCount(), 1);

    EXPECT_EQ(mixxx::SearchQueriesStorage::loadQueries(config()),
            QStringList({QStringLiteral("artist:foo hello world")}));
}

TEST_F(QmlRecentSearchTest, PersistSerializesKeyTokens) {
    mixxx::qml::QmlRecentSearchModel model(config());

    model.persist({token("Key", "key", "11d", 11)}, QString(), -1);

    EXPECT_EQ(mixxx::SearchQueriesStorage::loadQueries(config()),
            QStringList({QStringLiteral("key_id:11")}));
    const QVariantMap entry = model.get(0);
    EXPECT_QSTRING_EQ("key_id:11",
            entry.value(QStringLiteral("queryString")).toString());
    EXPECT_QSTRING_EQ("11",
            entry.value(QStringLiteral("tokens"))
                    .toList()
                    .at(0)
                    .toMap()
                    .value(QStringLiteral("keyId"))
                    .toString());
}

TEST_F(QmlRecentSearchTest, PersistReplacesActiveRow) {
    mixxx::SearchQueriesStorage::saveQueries(config(),
            {QStringLiteral("artist:foo"), QStringLiteral("title:bar")});

    mixxx::qml::QmlRecentSearchModel model(config());
    ASSERT_EQ(model.rowCount(), 2);

    EXPECT_EQ(model.persist({token("Artist", "artist", "baz", 0)}, QString(), 1), 1);
    EXPECT_EQ(model.rowCount(), 2);
    EXPECT_EQ(mixxx::SearchQueriesStorage::loadQueries(config()),
            QStringList({QStringLiteral("artist:foo"), QStringLiteral("artist:baz")}));
}

TEST_F(QmlRecentSearchTest, PersistRemovesDuplicateBeforePrepend) {
    mixxx::qml::QmlRecentSearchModel model(config());

    model.persist({token("Artist", "artist", "foo", 0)}, QString(), -1);
    model.persist({token("Title", "title", "bar", 0)}, QString(), -1);
    ASSERT_EQ(model.rowCount(), 2);

    // Re-persisting "artist:foo" moves it to the front instead of
    // accumulating a duplicate until the next reload.
    EXPECT_EQ(model.persist({token("Artist", "artist", "foo", 0)}, QString(), -1), 0);
    EXPECT_EQ(model.rowCount(), 2);
    EXPECT_QSTRING_EQ("artist:foo",
            model.get(0).value(QStringLiteral("queryString")).toString());
    EXPECT_QSTRING_EQ("title:bar",
            model.get(1).value(QStringLiteral("queryString")).toString());
    EXPECT_EQ(mixxx::SearchQueriesStorage::loadQueries(config()),
            QStringList({QStringLiteral("artist:foo"), QStringLiteral("title:bar")}));
}

TEST_F(QmlRecentSearchTest, PersistCapsListSize) {
    mixxx::qml::QmlRecentSearchModel model(config());

    for (int i = 0; i < mixxx::SearchQueriesStorage::kMaxQueries + 5; ++i) {
        model.persist({token("Artist", "artist", QString("v%1").arg(i), 0)},
                QString(),
                -1);
    }

    EXPECT_EQ(model.rowCount(), mixxx::SearchQueriesStorage::kMaxQueries);
    // Each persist inserts at the front, so the last persisted entry is
    // index 0 and the oldest surviving entry is at the back.
    EXPECT_QSTRING_EQ("artist:v54",
            model.get(0).value(QStringLiteral("queryString")).toString());
    EXPECT_QSTRING_EQ("artist:v5",
            model.get(mixxx::SearchQueriesStorage::kMaxQueries - 1)
                    .value(QStringLiteral("queryString"))
                    .toString());
}

TEST_F(QmlRecentSearchTest, PersistEmptyDoesNotStore) {
    mixxx::qml::QmlRecentSearchModel model(config());

    EXPECT_EQ(model.persist({}, QString(), 0), -1);
    EXPECT_EQ(model.rowCount(), 0);
    EXPECT_TRUE(mixxx::SearchQueriesStorage::loadQueries(config()).isEmpty());
}

} // namespace
