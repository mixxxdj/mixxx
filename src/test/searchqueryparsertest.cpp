#include <gtest/gtest.h>

#include <QDir>
#include <QtDebug>

#include "library/searchqueryparser.h"
#include "test/librarytest.h"
#include "track/track.h"
#include "util/assert.h"

TrackPointer newTestTrack(int sampleRate) {
    TrackPointer pTrack(Track::newTemporary());
    pTrack->setAudioProperties(
            mixxx::audio::ChannelCount(2),
            mixxx::audio::SampleRate(sampleRate),
            mixxx::audio::Bitrate(),
            mixxx::Duration::fromSeconds(180));
    return pTrack;
}

class SearchQueryParserTest : public LibraryTest {
  protected:
    SearchQueryParserTest()
            : m_parser(internalCollection()) {
    }

    virtual ~SearchQueryParserTest() {
    }

    TrackId addTrackToCollection(const QString& trackLocation) {
        TrackPointer pTrack =
                getOrAddTrackByLocation(trackLocation);
        return pTrack ? pTrack->getId() : TrackId();
    }

    SearchQueryParser m_parser;

    // The expected query to be returned by CrateFilterNode
    const QString m_crateFilterQuery =
        "id IN (SELECT DISTINCT track_id FROM crate_tracks"
        " JOIN crates ON crate_id=id WHERE name LIKE '%%1%' ORDER BY track_id)";
};

TEST_F(SearchQueryParserTest, EmptySearch) {
    auto pQuery(
        m_parser.parseQuery("", QStringList(), ""));

    // An empty query matches all tracks.
    TrackPointer pTrack(Track::newTemporary());
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(qPrintable(QString("")),
                 qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, OneTermOneColumn) {
    QStringList searchColumns;
    searchColumns << "artist";

    auto pQuery(
        m_parser.parseQuery("asdf", searchColumns, ""));

    TrackPointer pTrack(Track::newTemporary());
    pTrack->setTitle("testASDFtest");
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setArtist("testASDFtest");
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString("artist LIKE '%asdf%'")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, OneTermMultipleColumns) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";

    auto pQuery(
        m_parser.parseQuery("asdf", searchColumns, ""));

    TrackPointer pTrack(Track::newTemporary());
    pTrack->setTitle("testASDFtest");
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setAlbum("testASDFtest");
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString("(artist LIKE '%asdf%') OR (album LIKE '%asdf%')")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, OneTermMultipleColumnsNegation) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";

    auto pQuery(
        m_parser.parseQuery("-asdf", searchColumns, ""));

    TrackPointer pTrack(Track::newTemporary());
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

    auto pQuery(
        m_parser.parseQuery("asdf zxcv", searchColumns, ""));

    TrackPointer pTrack(Track::newTemporary());
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

    auto pQuery(
        m_parser.parseQuery("asdf zxcv", searchColumns, ""));

    TrackPointer pTrack(Track::newTemporary());
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

    auto pQuery(
        m_parser.parseQuery("asdf -zxcv", searchColumns, ""));

    TrackPointer pTrack(Track::newTemporary());
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
            "AND (NOT ((artist LIKE '%zxcv%') OR (album LIKE '%zxcv%')))")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, TextFilter) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";

    auto pQuery(
        m_parser.parseQuery("comment:asdf", searchColumns, ""));

    TrackPointer pTrack(Track::newTemporary());
    pTrack->setArtist("asdf");
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setComment("test ASDF test");
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString("comment LIKE '%asdf%'")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, TextFilterEmpty) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";

    // An empty argument should pass everything.
    auto pQuery(
        m_parser.parseQuery("comment:", searchColumns, ""));

    TrackPointer pTrack(Track::newTemporary());
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

    auto pQuery(
        m_parser.parseQuery("comment:\"asdf zxcv\"", searchColumns, ""));

    TrackPointer pTrack(Track::newTemporary());
    pTrack->setArtist("asdf zxcv");
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setComment("test ASDF zxcv test");
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString("comment LIKE '%asdf zxcv%'")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, TextFilterQuote_NoEndQuoteTakesWholeQuery) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";

    auto pQuery(
        m_parser.parseQuery("comment:\"asdf zxcv qwer", searchColumns, ""));

    TrackPointer pTrack(Track::newTemporary());
    pTrack->setArtist("asdf zxcv qwer");
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setComment("test ASDF zxcv qwer test");
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString("comment LIKE '%asdf zxcv qwer%'")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, TextFilterAllowsSpace) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";

    auto pQuery(
        m_parser.parseQuery("comment: asdf", searchColumns, ""));

    TrackPointer pTrack(Track::newTemporary());
    pTrack->setArtist("asdf");
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setComment("test ASDF test");
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString("comment LIKE '%asdf%'")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, TextFilterQuotes) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";

    auto pQuery(
        m_parser.parseQuery("comment:\"asdf ewe\"", searchColumns, ""));

    TrackPointer pTrack(Track::newTemporary());
    pTrack->setArtist("asdf");
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setComment("test ASDF ewetest");
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString("comment LIKE '%asdf ewe%'")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, TextFilterDecoration) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";

    auto pQuery(
        m_parser.parseQuery(QString::fromUtf8("comment:\"asdf\xC2\xB0 ewe\""), searchColumns, ""));  // with ˚

    TrackPointer pTrack(Track::newTemporary());
    pTrack->setArtist("asdf");
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setComment("test ASDF  ewetest");
    EXPECT_FALSE(pQuery->match(pTrack));

    pTrack->setComment(QString::fromUtf8("comment:\"asdf\xC2\xB0 ewe\""));
    EXPECT_TRUE(pQuery->match(pTrack));

    qDebug() << pQuery->toSql();

    EXPECT_STREQ(
        qPrintable(QString::fromUtf8("comment LIKE '%asdf\xC2\xB0 ewe%'")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, TextFilterTrailingSpace) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";

    auto pQuery(
        m_parser.parseQuery("comment:\"asdf \"", searchColumns, ""));

    TrackPointer pTrack(Track::newTemporary());
    pTrack->setArtist("asdf");
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setComment("test ASDF test");
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString("comment LIKE '%asdf _%'")),
        qPrintable(pQuery->toSql()));

    // We allow to search for two consequitve spaces
    auto pQuery2(
        m_parser.parseQuery("comment:\"  \"", searchColumns, ""));

    EXPECT_FALSE(pQuery2->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString("comment LIKE '%  _%'")),
        qPrintable(pQuery2->toSql()));
}

TEST_F(SearchQueryParserTest, TextFilterNegation) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";

    auto pQuery(
        m_parser.parseQuery("-comment: asdf", searchColumns, ""));

    TrackPointer pTrack(Track::newTemporary());
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

    auto pQuery(
        m_parser.parseQuery("bpm:127.12", searchColumns, ""));

    TrackPointer pTrack = newTestTrack(44100);
    pTrack->setBpm(127);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setBpm(127.12);
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString("bpm = 127.12")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, NumericFilterEmpty) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";

    auto pQuery(
        m_parser.parseQuery("bpm:", searchColumns, ""));

    TrackPointer pTrack = newTestTrack(44100);
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

    auto pQuery(
        m_parser.parseQuery("-bpm:127.12", searchColumns, ""));

    TrackPointer pTrack = newTestTrack(44100);
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

    auto pQuery(
        m_parser.parseQuery("bpm: 127.12", searchColumns, ""));

    TrackPointer pTrack = newTestTrack(44100);
    pTrack->setBpm(127);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setBpm(127.12);
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString("bpm = 127.12")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, NumericFilterOperators) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";

    auto pQuery(
        m_parser.parseQuery("bpm:>127.12", searchColumns, ""));

    TrackPointer pTrack = newTestTrack(44100);
    pTrack->setBpm(127.12);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setBpm(127.13);
    EXPECT_TRUE(pQuery->match(pTrack));
    EXPECT_STREQ(
        qPrintable(QString("bpm > 127.12")),
        qPrintable(pQuery->toSql()));


    pQuery = m_parser.parseQuery("bpm:>=127.12", searchColumns, "");
    pTrack->setBpm(127.11);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setBpm(127.12);
    EXPECT_TRUE(pQuery->match(pTrack));
    EXPECT_STREQ(
        qPrintable(QString("bpm >= 127.12")),
        qPrintable(pQuery->toSql()));

    pQuery = m_parser.parseQuery("bpm:<127.12", searchColumns, "");
    pTrack->setBpm(127.12);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setBpm(127.11);
    EXPECT_TRUE(pQuery->match(pTrack));
    EXPECT_STREQ(
        qPrintable(QString("bpm < 127.12")),
        qPrintable(pQuery->toSql()));

    pQuery = m_parser.parseQuery("bpm:<=127.12", searchColumns, "");
    pTrack->setBpm(127.13);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setBpm(127.12);
    EXPECT_TRUE(pQuery->match(pTrack));
    EXPECT_STREQ(
        qPrintable(QString("bpm <= 127.12")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, NumericRangeFilter) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";

    auto pQuery(
        m_parser.parseQuery("bpm:127.12-129", searchColumns, ""));

    TrackPointer pTrack = newTestTrack(44100);
    pTrack->setBpm(125);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setBpm(127.12);
    EXPECT_TRUE(pQuery->match(pTrack));
    pTrack->setBpm(129);
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString("(bpm >= 127.12) AND (bpm <= 129)")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, MultipleFilters) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "title";

    auto pQuery(
        m_parser.parseQuery("bpm:127.12-129 artist:\"com truise\" Colorvision",
                            searchColumns, ""));

    TrackPointer pTrack = newTestTrack(44100);
    pTrack->setBpm(128);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setArtist("Com Truise");
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setTitle("Colorvision");
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString("((bpm >= 127.12) AND (bpm <= 129)) AND "
                           "((artist LIKE '%com truise%') OR (album_artist LIKE '%com truise%')) AND "
                           "((artist LIKE '%colorvision%') OR (title LIKE '%colorvision%'))")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, ExtraFilterAppended) {
    QStringList searchColumns;
    searchColumns << "artist";

    auto pQuery(
        m_parser.parseQuery("asdf", searchColumns, "1 > 2"));

    TrackPointer pTrack = newTestTrack(44100);
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

    auto pQuery(
        m_parser.parseQuery("duration:1:30", searchColumns, ""));

    TrackPointer pTrack = newTestTrack(44100);
    pTrack->setDuration(91);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(90);
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString("duration = 90")),
        qPrintable(pQuery->toSql()));

    pQuery = m_parser.parseQuery("duration:1m30s", searchColumns, "");
    pTrack->setDuration(91);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(90);
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString("duration = 90")),
        qPrintable(pQuery->toSql()));

    pQuery = m_parser.parseQuery("duration:90", searchColumns, "");
    pTrack->setDuration(91);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(90);
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString("duration = 90")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, HumanReadableDurationSearchWithOperators) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";

    auto pQuery(
        m_parser.parseQuery("duration:>1:30", searchColumns, ""));

    TrackPointer pTrack = newTestTrack(44100);
    pTrack->setDuration(89);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(91);
    EXPECT_TRUE(pQuery->match(pTrack));
    EXPECT_STREQ(
        qPrintable(QString("duration > 90")),
        qPrintable(pQuery->toSql()));

    pQuery = m_parser.parseQuery("duration:>=90", searchColumns, "");
    pTrack->setDuration(89);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(90);
    EXPECT_TRUE(pQuery->match(pTrack));
    EXPECT_STREQ(
        qPrintable(QString("duration >= 90")),
        qPrintable(pQuery->toSql()));

    pQuery = m_parser.parseQuery("duration:>=1:30", searchColumns, "");
    pTrack->setDuration(89);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(90);
    EXPECT_TRUE(pQuery->match(pTrack));
    EXPECT_STREQ(
        qPrintable(QString("duration >= 90")),
        qPrintable(pQuery->toSql()));

    pQuery = m_parser.parseQuery("duration:<2:30", searchColumns, "");
    pTrack->setDuration(151);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(89);
    EXPECT_TRUE(pQuery->match(pTrack));
    EXPECT_STREQ(
        qPrintable(QString("duration < 150")),
        qPrintable(pQuery->toSql()));

    pQuery = m_parser.parseQuery("duration:<=2:30", searchColumns, "");
    pTrack->setDuration(191);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(150);
    EXPECT_TRUE(pQuery->match(pTrack));
    EXPECT_STREQ(
        qPrintable(QString("duration <= 150")),
        qPrintable(pQuery->toSql()));

    pQuery = m_parser.parseQuery("duration:<=150", searchColumns, "");
    pTrack->setDuration(191);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(150);
    EXPECT_TRUE(pQuery->match(pTrack));
    EXPECT_STREQ(
        qPrintable(QString("duration <= 150")),
        qPrintable(pQuery->toSql()));

    pQuery = m_parser.parseQuery("duration:<=2m30s", searchColumns, "");
    pTrack->setDuration(191);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(150);
    EXPECT_TRUE(pQuery->match(pTrack));
    EXPECT_STREQ(
        qPrintable(QString("duration <= 150")),
        qPrintable(pQuery->toSql()));

    pQuery = m_parser.parseQuery("duration:<=2m", searchColumns, "");
    pTrack->setDuration(191);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(110);
    EXPECT_TRUE(pQuery->match(pTrack));
    EXPECT_STREQ(
        qPrintable(QString("duration <= 120")),
        qPrintable(pQuery->toSql()));

    pQuery = m_parser.parseQuery("duration:<=2:", searchColumns, "");
    pTrack->setDuration(191);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(110);
    EXPECT_TRUE(pQuery->match(pTrack));
    EXPECT_STREQ(
        qPrintable(QString("duration <= 120")),
        qPrintable(pQuery->toSql()));

    pQuery = m_parser.parseQuery("duration:>=1:3", searchColumns, "");
    pTrack->setDuration(60);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(150);
    EXPECT_TRUE(pQuery->match(pTrack));
    EXPECT_STREQ(
        qPrintable(QString("duration >= 63")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, HumanReadableDurationSearchwithRangeFilter) {
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";

    auto pQuery(
        m_parser.parseQuery("duration:2:30-3:20", searchColumns, ""));

    TrackPointer pTrack = newTestTrack(44100);
    pTrack->setDuration(80);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(150);
    EXPECT_TRUE(pQuery->match(pTrack));
    pTrack->setDuration(199);
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString("(duration >= 150) AND (duration <= 200)")),
        qPrintable(pQuery->toSql()));

    pQuery = m_parser.parseQuery("duration:2:30-200", searchColumns, "");
    pTrack->setDuration(80);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(150);
    EXPECT_TRUE(pQuery->match(pTrack));
    pTrack->setDuration(199);
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString("(duration >= 150) AND (duration <= 200)")),
        qPrintable(pQuery->toSql()));

    pQuery = m_parser.parseQuery("duration:150-200", searchColumns, "");
    pTrack->setDuration(80);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(150);
    EXPECT_TRUE(pQuery->match(pTrack));
    pTrack->setDuration(199);
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString("(duration >= 150) AND (duration <= 200)")),
        qPrintable(pQuery->toSql()));

    pQuery = m_parser.parseQuery("duration:2m30s-3m20s", searchColumns, "");
    pTrack->setDuration(80);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(150);
    EXPECT_TRUE(pQuery->match(pTrack));
    pTrack->setDuration(199);
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString("(duration >= 150) AND (duration <= 200)")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, CrateFilter) {
    // User's search term
    QString searchTerm = "test";

    // Parse the user query
    auto pQuery(m_parser.parseQuery(QString("crate: %1").arg(searchTerm),
                                    QStringList(), ""));

    // locations for test tracks
    const QString kTrackALocationTest(QDir::currentPath() %
                  "/src/test/id3-test-data/cover-test-jpg.mp3");
    const QString kTrackBLocationTest(QDir::currentPath() %
                  "/src/test/id3-test-data/cover-test-png.mp3");

    // Create new crate and add it to the collection
    Crate testCrate;
    testCrate.setName(searchTerm);
    CrateId testCrateId;
    internalCollection()->insertCrate(testCrate, &testCrateId);

    // Add the track in the collection
    TrackId trackAId = addTrackToCollection(kTrackALocationTest);
    TrackPointer pTrackA(Track::newDummy(kTrackALocationTest, trackAId));
    TrackId trackBId = addTrackToCollection(kTrackBLocationTest);
    TrackPointer pTrackB(Track::newDummy(kTrackBLocationTest, trackBId));

    // Add track A to the newly created crate
    QList<TrackId> trackIds;
    trackIds << trackAId;
    internalCollection()->addCrateTracks(testCrateId, trackIds);

    EXPECT_TRUE(pQuery->match(pTrackA));
    EXPECT_FALSE(pQuery->match(pTrackB));

    EXPECT_STREQ(
                 qPrintable(m_crateFilterQuery.arg(searchTerm)),
                 qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, ShortCrateFilter) {
    // User's search term
    QString crateName = "somecrate";
    QString searchTerm = "ecrat";
    QStringList searchColumns;
    searchColumns << "crate"
                  << "artist"
                  << "comment";

    // Parse the user query
    auto pQuery(m_parser.parseQuery(QString("%1").arg(searchTerm),
                                    searchColumns, ""));

    // locations for test tracks
    const QString kTrackALocationTest(QDir::currentPath() %
                  "/src/test/id3-test-data/cover-test-jpg.mp3");
    const QString kTrackBLocationTest(QDir::currentPath() %
                  "/src/test/id3-test-data/cover-test-png.mp3");
    const QString kTrackCLocationTest(QDir::currentPath() %
                  "/src/test/id3-test-data/artist.mp3");

    // Create new crate and add it to the collection
    Crate testCrate;
    testCrate.setName(crateName);
    CrateId testCrateId;
    internalCollection()->insertCrate(testCrate, &testCrateId);

    // Add the track in the collection
    TrackId trackAId = addTrackToCollection(kTrackALocationTest);
    TrackPointer pTrackA(Track::newDummy(kTrackALocationTest, trackAId));
    TrackId trackBId = addTrackToCollection(kTrackBLocationTest);
    TrackPointer pTrackB(Track::newDummy(kTrackBLocationTest, trackBId));
    TrackId trackCId = addTrackToCollection(kTrackCLocationTest);
    TrackPointer pTrackC(Track::newDummy(kTrackCLocationTest, trackCId));
    pTrackC->setComment("garbage somecrate garbage");

    // Add track A to the newly created crate
    QList<TrackId> trackIds;
    trackIds << trackAId;
    internalCollection()->addCrateTracks(testCrateId, trackIds);

    EXPECT_TRUE(pQuery->match(pTrackA));
    EXPECT_FALSE(pQuery->match(pTrackB));
    EXPECT_TRUE(pQuery->match(pTrackC));
}


TEST_F(SearchQueryParserTest, CrateFilterEmpty) {
    // Empty should match everything
    auto pQuery(m_parser.parseQuery(QString("crate: "), QStringList(), ""));

    TrackPointer pTrackA(Track::newTemporary());

    EXPECT_TRUE(pQuery->match(pTrackA));

    EXPECT_STREQ(
                 qPrintable(""),
                 qPrintable(pQuery->toSql()));
}

// Checks if the crate filter works with quoted text with whitespaces
TEST_F(SearchQueryParserTest, CrateFilterQuote){
    // User's search term
    QString searchTerm = "test with whitespace";

    // Parse the user query
    auto pQuery(m_parser.parseQuery(QString("crate: \"%1\"").arg(searchTerm),
                                    QStringList(), ""));

    // locations for test tracks
    const QString kTrackALocationTest(QDir::currentPath() %
                  "/src/test/id3-test-data/cover-test-jpg.mp3");
    const QString kTrackBLocationTest(QDir::currentPath() %
                  "/src/test/id3-test-data/cover-test-png.mp3");

    // Create new crate and add it to the collection
    Crate testCrate;
    testCrate.setName(searchTerm);
    CrateId testCrateId;
    internalCollection()->insertCrate(testCrate, &testCrateId);

    // Add the tracks in the collection
    TrackId trackAId = addTrackToCollection(kTrackALocationTest);
    TrackPointer pTrackA(Track::newDummy(kTrackALocationTest, trackAId));
    TrackId trackBId = addTrackToCollection(kTrackBLocationTest);
    TrackPointer pTrackB(Track::newDummy(kTrackBLocationTest, trackBId));

    // Add track A to the newly created crate
    QList<TrackId> trackIds;
    trackIds << trackAId;
    internalCollection()->addCrateTracks(testCrateId, trackIds);

    EXPECT_TRUE(pQuery->match(pTrackA));
    EXPECT_FALSE(pQuery->match(pTrackB));

    EXPECT_STREQ(
                 qPrintable(m_crateFilterQuery.arg(searchTerm)),
                 qPrintable(pQuery->toSql()));
}

// Tests if the crate filter works along with other filters (artist)
TEST_F(SearchQueryParserTest, CrateFilterWithOther){
    QStringList searchColumns;
    searchColumns << "artist"
                  << "album";

    // User's search term
    QString searchTerm = "test";

    // Parse the user query
    auto pQuery(m_parser.parseQuery(QString("crate: %1 artist: asdf").arg(searchTerm),
                                    QStringList(), ""));

    // locations for test tracks
    const QString kTrackALocationTest(QDir::currentPath() %
                  "/src/test/id3-test-data/cover-test-jpg.mp3");
    const QString kTrackBLocationTest(QDir::currentPath() %
                  "/src/test/id3-test-data/cover-test-png.mp3");

    // Create new crate and add it to the collection
    Crate testCrate;
    testCrate.setName(searchTerm);
    CrateId testCrateId;
    internalCollection()->insertCrate(testCrate, &testCrateId);

    // Add the tracks in the collection
    TrackId trackAId = addTrackToCollection(kTrackALocationTest);
    TrackPointer pTrackA(Track::newDummy(kTrackALocationTest, trackAId));
    TrackId trackBId = addTrackToCollection(kTrackBLocationTest);
    TrackPointer pTrackB(Track::newDummy(kTrackBLocationTest, trackBId));

    // Add trackA to the newly created crate
    QList<TrackId> trackIds;
    trackIds << trackAId;
    internalCollection()->addCrateTracks(testCrateId, trackIds);

    pTrackA->setArtist("asdf");
    pTrackB->setArtist("asdf");

    EXPECT_TRUE(pQuery->match(pTrackA));
    EXPECT_FALSE(pQuery->match(pTrackB));

    EXPECT_STREQ(
                 qPrintable("(" + m_crateFilterQuery.arg(searchTerm) +
                            ") AND ((artist LIKE '%asdf%') OR (album_artist LIKE '%asdf%'))"),
                 qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, CrateFilterWithCrateFilterAndNegation){
    // User's search term
    QString searchTermA = "testA'1"; // Also a test if "'" is escaped lp1789728
    QString searchTermAEsc = "testA''1";
    QString searchTermB = "testB";

    // Parse the user query
    auto pQueryA(m_parser.parseQuery(QString("crate: %1 crate: %2").arg(searchTermA, searchTermB),
                                    QStringList(), ""));

    // locations for test tracks
    const QString kTrackALocationTest(QDir::currentPath() %
                  "/src/test/id3-test-data/cover-test-jpg.mp3");
    const QString kTrackBLocationTest(QDir::currentPath() %
                  "/src/test/id3-test-data/cover-test-png.mp3");

    // Create new crates and add them to the collection
    Crate testCrateA;
    testCrateA.setName(searchTermA);
    CrateId testCrateAId;
    internalCollection()->insertCrate(testCrateA, &testCrateAId);
    Crate testCrateB;
    testCrateB.setName(searchTermB);
    CrateId testCrateBId;
    internalCollection()->insertCrate(testCrateB, &testCrateBId);

    // Add the tracks in the collection
    TrackId trackAId = addTrackToCollection(kTrackALocationTest);
    TrackPointer pTrackA(Track::newDummy(kTrackALocationTest, trackAId));
    TrackId trackBId = addTrackToCollection(kTrackBLocationTest);
    TrackPointer pTrackB(Track::newDummy(kTrackBLocationTest, trackBId));

    // Add trackA and trackB to crate A
    QList<TrackId> trackIdsA;
    trackIdsA << trackAId << trackBId;
    internalCollection()->addCrateTracks(testCrateAId, trackIdsA);

    // Add trackA to crate B
    QList<TrackId> trackIdsB;
    trackIdsB << trackAId;
    internalCollection()->addCrateTracks(testCrateBId, trackIdsB);

    EXPECT_TRUE(pQueryA->match(pTrackA));
    EXPECT_FALSE(pQueryA->match(pTrackB));

    EXPECT_STREQ(
                 qPrintable("(" + m_crateFilterQuery.arg(searchTermAEsc) +
                            ") AND (" + m_crateFilterQuery.arg(searchTermB) + ")"),
                 qPrintable(pQueryA->toSql()));

    // parse again to test negation
    auto pQueryB(m_parser.parseQuery(QString("crate: %1 -crate: %2").arg(searchTermA, searchTermB),
                                     QStringList(), ""));

    EXPECT_FALSE(pQueryB->match(pTrackA));
    EXPECT_TRUE(pQueryB->match(pTrackB));

    EXPECT_STREQ(
                 qPrintable("(" + m_crateFilterQuery.arg(searchTermAEsc) +
                            ") AND (NOT (" + m_crateFilterQuery.arg(searchTermB) + "))"),
                 qPrintable(pQueryB->toSql()));
}
