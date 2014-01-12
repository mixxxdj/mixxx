#include <gtest/gtest.h>
#include <QtDebug>
#include <QDir>
#include <QTemporaryFile>

#include "library/searchqueryparser.h"

class SearchQueryParserTest : public testing::Test {
  protected:
    SearchQueryParserTest()
            : m_database(QSqlDatabase::addDatabase("QSQLITE")),
              m_parser(m_database) {
        QTemporaryFile databaseFile("mixxxdb.sqlite");
        Q_ASSERT(databaseFile.open());
        m_database.setHostName("localhost");
        m_database.setUserName("mixxx");
        m_database.setPassword("mixxx");
        qDebug() << "Temp file is" << databaseFile.fileName();
        m_database.setDatabaseName(databaseFile.fileName());
        Q_ASSERT(m_database.open());
    }

    virtual ~SearchQueryParserTest() {
    }

    QSqlDatabase m_database;
    SearchQueryParser m_parser;
};

TEST_F(SearchQueryParserTest, EmptySearch) {
    QScopedPointer<QueryNode> pQuery(
        m_parser.parseQuery("", QStringList(), ""));
    EXPECT_STREQ(qPrintable(QString("")),
                 qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, OneTermOneColumn) {
    QStringList searchColumns;
    searchColumns << "artist";

    QScopedPointer<QueryNode> pQuery(
        m_parser.parseQuery("asdf", searchColumns, ""));

    EXPECT_STREQ(
        qPrintable(QString("(artist LIKE '%asdf%')")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, OneTermMultipleColumns) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";

    QScopedPointer<QueryNode> pQuery(
        m_parser.parseQuery("asdf", searchColumns, ""));

    EXPECT_STREQ(
        qPrintable(QString("((artist LIKE '%asdf%') OR (album LIKE '%asdf%'))")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, MultipleTermsOneColumn) {
    QStringList searchColumns;
    searchColumns << "artist";

    QScopedPointer<QueryNode> pQuery(
        m_parser.parseQuery("asdf zxcv", searchColumns, ""));

    EXPECT_STREQ(
        qPrintable(QString("(artist LIKE '%asdf%') AND (artist LIKE '%zxcv%')")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, MultipleTermsMultipleColumns) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";

    QScopedPointer<QueryNode> pQuery(
        m_parser.parseQuery("asdf zxcv", searchColumns, ""));

    EXPECT_STREQ(
        qPrintable(QString(
            "((artist LIKE '%asdf%') OR (album LIKE '%asdf%')) "
            "AND ((artist LIKE '%zxcv%') OR (album LIKE '%zxcv%'))")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, TextFilter) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";

    QScopedPointer<QueryNode> pQuery(
        m_parser.parseQuery("comment:asdf", searchColumns, ""));

    EXPECT_STREQ(
        qPrintable(QString("(comment LIKE '%asdf%')")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, TextFilterQuote) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";

    QScopedPointer<QueryNode> pQuery(
        m_parser.parseQuery("comment:\"asdf zxcv\"", searchColumns, ""));

    EXPECT_STREQ(
        qPrintable(QString("(comment LIKE '%asdf zxcv%')")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, TextFilterQuote_NoEndQuoteTakesWholeQuery) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";

    QScopedPointer<QueryNode> pQuery(
        m_parser.parseQuery("comment:\"asdf zxcv qwer", searchColumns, ""));

    EXPECT_STREQ(
        qPrintable(QString("(comment LIKE '%asdf zxcv qwer%')")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, TextFilterAllowsSpace) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";

    QScopedPointer<QueryNode> pQuery(
        m_parser.parseQuery("comment: asdf", searchColumns, ""));

    EXPECT_STREQ(
        qPrintable(QString("(comment LIKE '%asdf%')")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, NumericFilter) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";

    QScopedPointer<QueryNode> pQuery(
        m_parser.parseQuery("bpm:127.12", searchColumns, ""));

    EXPECT_STREQ(
        qPrintable(QString("(bpm = 127.12)")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, NumericFilterAllowsSpace) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";

    QScopedPointer<QueryNode> pQuery(
        m_parser.parseQuery("bpm: 127.12", searchColumns, ""));

    EXPECT_STREQ(
        qPrintable(QString("(bpm = 127.12)")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, NumericFilterOperators) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";

    QScopedPointer<QueryNode> pQuery(
        m_parser.parseQuery("bpm:>127.12", searchColumns, ""));
    EXPECT_STREQ(
        qPrintable(QString("(bpm > 127.12)")),
        qPrintable(pQuery->toSql()));


    pQuery.reset(m_parser.parseQuery("bpm:>=127.12", searchColumns, ""));
    EXPECT_STREQ(
        qPrintable(QString("(bpm >= 127.12)")),
        qPrintable(pQuery->toSql()));

    pQuery.reset(m_parser.parseQuery("bpm:<127.12", searchColumns, ""));
    EXPECT_STREQ(
        qPrintable(QString("(bpm < 127.12)")),
        qPrintable(pQuery->toSql()));

    pQuery.reset(m_parser.parseQuery("bpm:<=127.12", searchColumns, ""));
    EXPECT_STREQ(
        qPrintable(QString("(bpm <= 127.12)")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, NumericRangeFilter) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";

    QScopedPointer<QueryNode> pQuery(
        m_parser.parseQuery("bpm:127.12-129", searchColumns, ""));

    EXPECT_STREQ(
        qPrintable(QString("(bpm >= 127.12 AND bpm <= 129)")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, MultipleFilters) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "title";

    QScopedPointer<QueryNode> pQuery(
        m_parser.parseQuery("bpm:127.12-129 artist:\"com truise\" Colorvision",
                            searchColumns, ""));

    EXPECT_STREQ(
        qPrintable(QString("(bpm >= 127.12 AND bpm <= 129) AND "
                           "((artist LIKE '%com truise%') OR (album_artist LIKE '%com truise%')) AND "
                           "((artist LIKE '%Colorvision%') OR (title LIKE '%Colorvision%'))")),
        qPrintable(pQuery->toSql()));
}


TEST_F(SearchQueryParserTest, ExtraFilterAppended) {
    QStringList searchColumns;
    searchColumns << "artist";

    QScopedPointer<QueryNode> pQuery(
        m_parser.parseQuery("asdf", searchColumns, "1 > 2"));

    EXPECT_STREQ(
        qPrintable(QString("(1 > 2) AND (artist LIKE '%asdf%')")),
        qPrintable(pQuery->toSql()));
}
