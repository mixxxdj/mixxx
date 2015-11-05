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

    // An empty query matches all tracks.
    TrackPointer pTrack(new TrackInfoObject());
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(qPrintable(QString("")),
                 qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, OneTermOneColumn) {
    QStringList searchColumns;
    searchColumns << "artist";

    QScopedPointer<QueryNode> pQuery(
        m_parser.parseQuery("asdf", searchColumns, ""));

    TrackPointer pTrack(new TrackInfoObject());
    pTrack->setTitle("testASDFtest");
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setArtist("testASDFtest");
    EXPECT_TRUE(pQuery->match(pTrack));

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

    TrackPointer pTrack(new TrackInfoObject());
    pTrack->setTitle("testASDFtest");
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setAlbum("testASDFtest");
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString("((artist LIKE '%asdf%') OR (album LIKE '%asdf%'))")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, OneTermMultipleColumnsNegation) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";

    QScopedPointer<QueryNode> pQuery(
        m_parser.parseQuery("-asdf", searchColumns, ""));

    TrackPointer pTrack(new TrackInfoObject());
    pTrack->setTitle("testASDFtest");
    EXPECT_TRUE(pQuery->match(pTrack));
    pTrack->setAlbum("testASDFtest");
    EXPECT_FALSE(pQuery->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString("NOT ((artist LIKE '%asdf%') OR (album LIKE '%asdf%'))")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, MultipleTermsOneColumn) {
    QStringList searchColumns;
    searchColumns << "artist";

    QScopedPointer<QueryNode> pQuery(
        m_parser.parseQuery("asdf zxcv", searchColumns, ""));

    TrackPointer pTrack(new TrackInfoObject());
    pTrack->setTitle("test zXcV test");
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setArtist("test zXcV test asDf");
    EXPECT_TRUE(pQuery->match(pTrack));

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

    TrackPointer pTrack(new TrackInfoObject());
    pTrack->setTitle("asdf zxcv");
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setArtist("zXcV");
    pTrack->setAlbum("asDF");
    EXPECT_TRUE(pQuery->match(pTrack));
    pTrack->setArtist("");
    pTrack->setAlbum("ASDF ZXCV");
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString(
            "((artist LIKE '%asdf%') OR (album LIKE '%asdf%')) "
            "AND ((artist LIKE '%zxcv%') OR (album LIKE '%zxcv%'))")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, MultipleTermsMultipleColumnsNegation) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";

    QScopedPointer<QueryNode> pQuery(
        m_parser.parseQuery("asdf -zxcv", searchColumns, ""));

    TrackPointer pTrack(new TrackInfoObject());
    pTrack->setTitle("asdf zxcv");
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setAlbum("asDF");
    EXPECT_TRUE(pQuery->match(pTrack));
    pTrack->setArtist("zXcV");
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setArtist("");
    pTrack->setAlbum("ASDF ZXCV");
    EXPECT_FALSE(pQuery->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString(
            "((artist LIKE '%asdf%') OR (album LIKE '%asdf%')) "
            "AND NOT ((artist LIKE '%zxcv%') OR (album LIKE '%zxcv%'))")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, TextFilter) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";

    QScopedPointer<QueryNode> pQuery(
        m_parser.parseQuery("comment:asdf", searchColumns, ""));

    TrackPointer pTrack(new TrackInfoObject());
    pTrack->setArtist("asdf");
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setComment("test ASDF test");
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString("(comment LIKE '%asdf%')")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, TextFilterEmpty) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";

    // An empty argument should pass everything.
    QScopedPointer<QueryNode> pQuery(
        m_parser.parseQuery("comment:", searchColumns, ""));

    TrackPointer pTrack(new TrackInfoObject());
    pTrack->setComment("test ASDF test");
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString("")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, TextFilterQuote) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";

    QScopedPointer<QueryNode> pQuery(
        m_parser.parseQuery("comment:\"asdf zxcv\"", searchColumns, ""));

    TrackPointer pTrack(new TrackInfoObject());
    pTrack->setArtist("asdf zxcv");
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setComment("test ASDF zxcv test");
    EXPECT_TRUE(pQuery->match(pTrack));

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

    TrackPointer pTrack(new TrackInfoObject());
    pTrack->setArtist("asdf zxcv qwer");
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setComment("test ASDF zxcv qwer test");
    EXPECT_TRUE(pQuery->match(pTrack));

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

    TrackPointer pTrack(new TrackInfoObject());
    pTrack->setArtist("asdf");
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setComment("test ASDF test");
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString("(comment LIKE '%asdf%')")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, TextFilterNegation) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";

    QScopedPointer<QueryNode> pQuery(
        m_parser.parseQuery("-comment: asdf", searchColumns, ""));

    TrackPointer pTrack(new TrackInfoObject());
    pTrack->setArtist("asdf");
    EXPECT_TRUE(pQuery->match(pTrack));
    pTrack->setComment("test ASDF test");
    EXPECT_FALSE(pQuery->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString("NOT (comment LIKE '%asdf%')")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, NumericFilter) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";

    QScopedPointer<QueryNode> pQuery(
        m_parser.parseQuery("bpm:127.12", searchColumns, ""));

    TrackPointer pTrack(new TrackInfoObject());
    pTrack->setSampleRate(44100);
    pTrack->setBpm(127);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setBpm(127.12);
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString("(bpm = 127.12)")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, NumericFilterEmpty) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";

    QScopedPointer<QueryNode> pQuery(
        m_parser.parseQuery("bpm:", searchColumns, ""));

    TrackPointer pTrack(new TrackInfoObject());
    pTrack->setSampleRate(44100);
    pTrack->setBpm(127);
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString("")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, NumericFilterNegation) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";

    QScopedPointer<QueryNode> pQuery(
        m_parser.parseQuery("-bpm:127.12", searchColumns, ""));

    TrackPointer pTrack(new TrackInfoObject());
    pTrack->setSampleRate(44100);
    pTrack->setBpm(127);
    EXPECT_TRUE(pQuery->match(pTrack));
    pTrack->setBpm(127.12);
    EXPECT_FALSE(pQuery->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString("NOT (bpm = 127.12)")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, NumericFilterAllowsSpace) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";

    QScopedPointer<QueryNode> pQuery(
        m_parser.parseQuery("bpm: 127.12", searchColumns, ""));

    TrackPointer pTrack(new TrackInfoObject());
    pTrack->setSampleRate(44100);
    pTrack->setBpm(127);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setBpm(127.12);
    EXPECT_TRUE(pQuery->match(pTrack));

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

    TrackPointer pTrack(new TrackInfoObject());
    pTrack->setSampleRate(44100);
    pTrack->setBpm(127.12);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setBpm(127.13);
    EXPECT_TRUE(pQuery->match(pTrack));
    EXPECT_STREQ(
        qPrintable(QString("(bpm > 127.12)")),
        qPrintable(pQuery->toSql()));


    pQuery.reset(m_parser.parseQuery("bpm:>=127.12", searchColumns, ""));
    pTrack->setBpm(127.11);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setBpm(127.12);
    EXPECT_TRUE(pQuery->match(pTrack));
    EXPECT_STREQ(
        qPrintable(QString("(bpm >= 127.12)")),
        qPrintable(pQuery->toSql()));

    pQuery.reset(m_parser.parseQuery("bpm:<127.12", searchColumns, ""));
    pTrack->setBpm(127.12);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setBpm(127.11);
    EXPECT_TRUE(pQuery->match(pTrack));
    EXPECT_STREQ(
        qPrintable(QString("(bpm < 127.12)")),
        qPrintable(pQuery->toSql()));

    pQuery.reset(m_parser.parseQuery("bpm:<=127.12", searchColumns, ""));
    pTrack->setBpm(127.13);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setBpm(127.12);
    EXPECT_TRUE(pQuery->match(pTrack));
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

    TrackPointer pTrack(new TrackInfoObject());
    pTrack->setSampleRate(44100);
    pTrack->setBpm(125);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setBpm(127.12);
    EXPECT_TRUE(pQuery->match(pTrack));
    pTrack->setBpm(129);
    EXPECT_TRUE(pQuery->match(pTrack));

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

    TrackPointer pTrack(new TrackInfoObject());
    pTrack->setSampleRate(44100);
    pTrack->setBpm(128);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setArtist("Com Truise");
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setTitle("Colorvision");
    EXPECT_TRUE(pQuery->match(pTrack));

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

    TrackPointer pTrack(new TrackInfoObject());
    pTrack->setArtist("zxcv");
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setArtist("asdf");
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString("(1 > 2) AND (artist LIKE '%asdf%')")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, HumanReadableDurationSearch) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";

    QScopedPointer<QueryNode> pQuery(
        m_parser.parseQuery("duration:1:30", searchColumns, ""));

    TrackPointer pTrack(new TrackInfoObject());
    pTrack->setSampleRate(44100);
    pTrack->setDuration(91);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(90);
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString("(duration = 90)")),
        qPrintable(pQuery->toSql()));

    pQuery.reset(m_parser.parseQuery("duration:1m30s", searchColumns, ""));
    pTrack->setDuration(91);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(90);
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString("(duration = 90)")),
        qPrintable(pQuery->toSql()));

    pQuery.reset(m_parser.parseQuery("duration:90", searchColumns, ""));
    pTrack->setDuration(91);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(90);
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString("(duration = 90)")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, HumanReadableDurationSearchwithOperators) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";

    QScopedPointer<QueryNode> pQuery(
        m_parser.parseQuery("duration:>1:30", searchColumns, ""));

    TrackPointer pTrack(new TrackInfoObject());
    pTrack->setSampleRate(44100);
    pTrack->setDuration(89);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(91);
    EXPECT_TRUE(pQuery->match(pTrack));
    EXPECT_STREQ(
        qPrintable(QString("(duration > 90)")),
        qPrintable(pQuery->toSql()));

    pQuery.reset(m_parser.parseQuery("duration:>=90", searchColumns, ""));
    pTrack->setDuration(89);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(90);
    EXPECT_TRUE(pQuery->match(pTrack));
    EXPECT_STREQ(
        qPrintable(QString("(duration >= 90)")),
        qPrintable(pQuery->toSql()));

    pQuery.reset(m_parser.parseQuery("duration:>=1:30", searchColumns, ""));
    pTrack->setDuration(89);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(90);
    EXPECT_TRUE(pQuery->match(pTrack));
    EXPECT_STREQ(
        qPrintable(QString("(duration >= 90)")),
        qPrintable(pQuery->toSql()));

    pQuery.reset(m_parser.parseQuery("duration:<2:30", searchColumns, ""));
    pTrack->setDuration(151);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(89);
    EXPECT_TRUE(pQuery->match(pTrack));
    EXPECT_STREQ(
        qPrintable(QString("(duration < 150)")),
        qPrintable(pQuery->toSql()));

    pQuery.reset(m_parser.parseQuery("duration:<=2:30", searchColumns, ""));
    pTrack->setDuration(191);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(150);
    EXPECT_TRUE(pQuery->match(pTrack));
    EXPECT_STREQ(
        qPrintable(QString("(duration <= 150)")),
        qPrintable(pQuery->toSql()));

    pQuery.reset(m_parser.parseQuery("duration:<=150", searchColumns, ""));
    pTrack->setDuration(191);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(150);
    EXPECT_TRUE(pQuery->match(pTrack));
    EXPECT_STREQ(
        qPrintable(QString("(duration <= 150)")),
        qPrintable(pQuery->toSql()));

    pQuery.reset(m_parser.parseQuery("duration:<=2m30s", searchColumns, ""));
    pTrack->setDuration(191);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(150);
    EXPECT_TRUE(pQuery->match(pTrack));
    EXPECT_STREQ(
        qPrintable(QString("(duration <= 150)")),
        qPrintable(pQuery->toSql()));

    pQuery.reset(m_parser.parseQuery("duration:<=2m", searchColumns, ""));
    pTrack->setDuration(191);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(110);
    EXPECT_TRUE(pQuery->match(pTrack));
    EXPECT_STREQ(
        qPrintable(QString("(duration <= 120)")),
        qPrintable(pQuery->toSql()));

    pQuery.reset(m_parser.parseQuery("duration:<=2:", searchColumns, ""));
    pTrack->setDuration(191);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(110);
    EXPECT_TRUE(pQuery->match(pTrack));
    EXPECT_STREQ(
        qPrintable(QString("(duration <= 120)")),
        qPrintable(pQuery->toSql()));

    pQuery.reset(m_parser.parseQuery("duration:>=1:3", searchColumns, ""));
    pTrack->setDuration(60);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(150);
    EXPECT_TRUE(pQuery->match(pTrack));
    EXPECT_STREQ(
        qPrintable(QString("(duration >= 63)")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, HumanReadableDurationSearchwithRangeFilter) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";

    QScopedPointer<QueryNode> pQuery(
        m_parser.parseQuery("duration:2:30-3:20", searchColumns, ""));

    TrackPointer pTrack(new TrackInfoObject());
    pTrack->setSampleRate(44100);
    pTrack->setDuration(80);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(150);
    EXPECT_TRUE(pQuery->match(pTrack));
    pTrack->setDuration(199);
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString("(duration >= 150 AND duration <= 200)")),
        qPrintable(pQuery->toSql()));

    pQuery.reset(m_parser.parseQuery("duration:2:30-200", searchColumns, ""));
    pTrack->setSampleRate(44100);
    pTrack->setDuration(80);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(150);
    EXPECT_TRUE(pQuery->match(pTrack));
    pTrack->setDuration(199);
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString("(duration >= 150 AND duration <= 200)")),
        qPrintable(pQuery->toSql()));

    pQuery.reset(m_parser.parseQuery("duration:150-200", searchColumns, ""));
    pTrack->setSampleRate(44100);
    pTrack->setDuration(80);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(150);
    EXPECT_TRUE(pQuery->match(pTrack));
    pTrack->setDuration(199);
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString("(duration >= 150 AND duration <= 200)")),
        qPrintable(pQuery->toSql()));

    pQuery.reset(m_parser.parseQuery("duration:2m30s-3m20s", searchColumns, ""));
    pTrack->setSampleRate(44100);
    pTrack->setDuration(80);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(150);
    EXPECT_TRUE(pQuery->match(pTrack));
    pTrack->setDuration(199);
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString("(duration >= 150 AND duration <= 200)")),
        qPrintable(pQuery->toSql()));
}
