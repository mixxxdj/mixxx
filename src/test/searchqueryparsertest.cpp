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
    EXPECT_STREQ(qPrintable(QString("")),
                 qPrintable(m_parser.parseQuery("", QStringList(), "")));
}

TEST_F(SearchQueryParserTest, OneTermOneColumn) {
    QStringList searchColumns;
    searchColumns << "artist";

    EXPECT_STREQ(
        qPrintable(QString("WHERE ((artist LIKE '%asdf%'))")),
        qPrintable(m_parser.parseQuery("asdf", searchColumns, "")));
}

TEST_F(SearchQueryParserTest, OneTermMultipleColumns) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";
    EXPECT_STREQ(
        qPrintable(QString("WHERE ((artist LIKE '%asdf%') OR (album LIKE '%asdf%'))")),
        qPrintable(m_parser.parseQuery("asdf", searchColumns, "")));
}

TEST_F(SearchQueryParserTest, MultipleTermsOneColumn) {
    QStringList searchColumns;
    searchColumns << "artist";

    EXPECT_STREQ(
        qPrintable(QString("WHERE ((artist LIKE '%asdf%')) AND ((artist LIKE '%zxcv%'))")),
        qPrintable(m_parser.parseQuery("asdf zxcv", searchColumns, "")));
}

TEST_F(SearchQueryParserTest, MultipleTermsMultipleColumns) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";
    EXPECT_STREQ(
        qPrintable(QString(
            "WHERE ((artist LIKE '%asdf%') OR (album LIKE '%asdf%')) "
            "AND ((artist LIKE '%zxcv%') OR (album LIKE '%zxcv%'))")),
        qPrintable(m_parser.parseQuery("asdf zxcv", searchColumns, "")));
}

TEST_F(SearchQueryParserTest, TextFilter) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";
    EXPECT_STREQ(
        qPrintable(QString("WHERE (comment LIKE '%asdf%')")),
        qPrintable(m_parser.parseQuery("comment:asdf", searchColumns, "")));
}

TEST_F(SearchQueryParserTest, TextFilterQuote) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";
    EXPECT_STREQ(
        qPrintable(QString("WHERE (comment LIKE '%asdf zxcv%')")),
        qPrintable(m_parser.parseQuery("comment:\"asdf zxcv\"", searchColumns, "")));
}

TEST_F(SearchQueryParserTest, TextFilterQuote_NoEndQuoteTakesWholeQuery) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";
    EXPECT_STREQ(
        qPrintable(QString("WHERE (comment LIKE '%asdf zxcv qwer%')")),
        qPrintable(m_parser.parseQuery("comment:\"asdf zxcv qwer", searchColumns, "")));
}

TEST_F(SearchQueryParserTest, TextFilterAllowsSpace) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";
    EXPECT_STREQ(
        qPrintable(QString("WHERE (comment LIKE '%asdf%')")),
        qPrintable(m_parser.parseQuery("comment: asdf", searchColumns, "")));
}

TEST_F(SearchQueryParserTest, NumericFilter) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";
    EXPECT_STREQ(
        qPrintable(QString("WHERE (bpm = 127.12)")),
        qPrintable(m_parser.parseQuery("bpm:127.12", searchColumns, "")));
}

TEST_F(SearchQueryParserTest, NumericFilterAllowsSpace) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";
    EXPECT_STREQ(
        qPrintable(QString("WHERE (bpm = 127.12)")),
        qPrintable(m_parser.parseQuery("bpm: 127.12", searchColumns, "")));
}

TEST_F(SearchQueryParserTest, NumericFilterOperators) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";
    EXPECT_STREQ(
        qPrintable(QString("WHERE (bpm > 127.12)")),
        qPrintable(m_parser.parseQuery("bpm:>127.12", searchColumns, "")));
    EXPECT_STREQ(
        qPrintable(QString("WHERE (bpm >= 127.12)")),
        qPrintable(m_parser.parseQuery("bpm:>=127.12", searchColumns, "")));
    EXPECT_STREQ(
        qPrintable(QString("WHERE (bpm < 127.12)")),
        qPrintable(m_parser.parseQuery("bpm:<127.12", searchColumns, "")));
    EXPECT_STREQ(
        qPrintable(QString("WHERE (bpm <= 127.12)")),
        qPrintable(m_parser.parseQuery("bpm:<=127.12", searchColumns, "")));
}

TEST_F(SearchQueryParserTest, NumericRangeFilter) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";
    EXPECT_STREQ(
        qPrintable(QString("WHERE (bpm >= 127.12 AND bpm <= 129)")),
        qPrintable(m_parser.parseQuery("bpm:127.12-129", searchColumns, "")));
}

TEST_F(SearchQueryParserTest, MultipleFilters) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "title";
    EXPECT_STREQ(
        qPrintable(QString("WHERE (bpm >= 127.12 AND bpm <= 129) AND "
                           "(artist LIKE '%com truise%') AND "
                           "((artist LIKE '%Colorvision%') OR (title LIKE '%Colorvision%'))")),
        qPrintable(m_parser.parseQuery("bpm:127.12-129 artist:\"com truise\" Colorvision",
                                       searchColumns, "")));
}


TEST_F(SearchQueryParserTest, ExtraFilterAppended) {
    QStringList searchColumns;
    searchColumns << "artist";
    EXPECT_STREQ(
        qPrintable(QString("WHERE (1 > 2) AND ((artist LIKE '%asdf%'))")),
        qPrintable(m_parser.parseQuery("asdf", searchColumns, "1 > 2")));
}
