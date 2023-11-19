#include <gtest/gtest.h>

#include <QDir>
#include <QtDebug>

#include "library/searchquery.h"
#include "library/searchqueryparser.h"
#include "library/trackset/crate/crate.h"
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
            : m_parser(internalCollection(), QStringList()) {
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
            m_parser.parseQuery("", QString()));

    // An empty query matches all tracks.
    TrackPointer pTrack(Track::newTemporary());
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(qPrintable(QString("")),
                 qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, OneTermOneColumn) {
    m_parser.setSearchColumns({"artist"});
    auto pQuery(
            m_parser.parseQuery("asdf", QString()));

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
    m_parser.setSearchColumns({"artist", "album"});
    auto pQuery(
            m_parser.parseQuery("asdf", QString()));

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
    m_parser.setSearchColumns({"artist", "album"});
    auto pQuery(
            m_parser.parseQuery("-asdf", QString()));

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
    m_parser.setSearchColumns({"artist"});
    auto pQuery(
            m_parser.parseQuery("asdf zxcv", QString()));

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
    m_parser.setSearchColumns({"artist", "album"});
    auto pQuery(
            m_parser.parseQuery("asdf zxcv", QString()));

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
    m_parser.setSearchColumns({"artist", "album"});
    auto pQuery(
            m_parser.parseQuery("asdf -zxcv", QString()));

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
    m_parser.setSearchColumns({"artist", "album"});
    auto pQuery(
            m_parser.parseQuery("comment:asdf", QString()));

    TrackPointer pTrack(Track::newTemporary());
    pTrack->setArtist("asdf");
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setComment("test ASDF test");
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString("comment LIKE '%asdf%'")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, TextFilterEquals) {
    m_parser.setSearchColumns({"artist", "album"});
    auto pQuery(m_parser.parseQuery("comment:=\"asdf\"", QString()));

    TrackPointer pTrack(Track::newTemporary());
    pTrack->setComment("test ASDF test");
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setComment("ASDF");
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
            qPrintable(QString("comment LIKE 'asdf'")),
            qPrintable(pQuery->toSql()));

    // Incomplete quoting should use StringMatch::Contains,
    // i.e. equal to 'comment:asdf'
    pQuery = m_parser.parseQuery("comment:=\"asdf", QString());

    pTrack->setComment("test ASDF test");
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
            qPrintable(QString("comment LIKE '%asdf%'")),
            qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, TextFilterEmpty) {
    m_parser.setSearchColumns({"artist", "album"});
    // An empty argument should pass everything.
    auto pQuery(
            m_parser.parseQuery("comment:", QString()));

    TrackPointer pTrack(Track::newTemporary());
    pTrack->setComment("test ASDF test");
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString("")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, TextFilterQuote) {
    m_parser.setSearchColumns({"artist", "album"});
    auto pQuery(
            m_parser.parseQuery("comment:\"asdf zxcv\"", QString()));

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
    m_parser.setSearchColumns({"artist", "album"});
    auto pQuery(
            m_parser.parseQuery("comment:\"asdf zxcv qwer", QString()));

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
    m_parser.setSearchColumns({"artist", "album"});
    auto pQuery(
            m_parser.parseQuery("comment: asdf", QString()));

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
    m_parser.setSearchColumns({"artist", "album"});
    auto pQuery(
            m_parser.parseQuery("comment:\"asdf ewe\"", QString()));

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
    m_parser.setSearchColumns({"artist", "album"});
    auto pQuery(m_parser.parseQuery(
            QString::fromUtf8("comment:\"asdf\xC2\xB0 ewe\""),
            QString())); // with ˚

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
    m_parser.setSearchColumns({"artist", "album"});
    auto pQuery(
            m_parser.parseQuery("comment:\"asdf \"", QString()));

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
            m_parser.parseQuery("comment:\"  \"", QString()));

    EXPECT_FALSE(pQuery2->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString("comment LIKE '%  _%'")),
        qPrintable(pQuery2->toSql()));
}

TEST_F(SearchQueryParserTest, TextFilterNegation) {
    m_parser.setSearchColumns({"artist", "album"});
    auto pQuery(
            m_parser.parseQuery("-comment: asdf", QString()));

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
    m_parser.setSearchColumns({"artist", "album"});
    auto pQuery(
            m_parser.parseQuery("bpm:127.12", QString()));

    TrackPointer pTrack = newTestTrack(44100);
    pTrack->trySetBpm(127);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->trySetBpm(127.12);
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString("bpm = 127.12")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, NumericFilterYear) {
    m_parser.setSearchColumns({"year"});

    auto pQuery(
            m_parser.parseQuery("year:1969", QString()));

    TrackPointer pTrack = newTestTrack(44100);
    EXPECT_FALSE(pQuery->match(pTrack));
    // Note: The sourounding spaces are checking that a user input is popperly trimmed.
    pTrack->setYear(" 1969-08-15 ");
    EXPECT_TRUE(pQuery->match(pTrack));
    pTrack->setYear(" 19690815 ");
    EXPECT_TRUE(pQuery->match(pTrack));
    pTrack->setYear(" 1969-extra ");
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
            qPrintable(QStringLiteral("CAST(substr(year,1,4) AS INTEGER) = 1969")),
            qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, NumericFilterEmpty) {
    m_parser.setSearchColumns({"artist", "album"});
    auto pQuery(
            m_parser.parseQuery("bpm:", QString()));

    TrackPointer pTrack = newTestTrack(44100);
    pTrack->trySetBpm(127);
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString("")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, NumericFilterNegation) {
    m_parser.setSearchColumns({"artist", "album"});
    auto pQuery(
            m_parser.parseQuery("-bpm:127.12", QString()));

    TrackPointer pTrack = newTestTrack(44100);
    pTrack->trySetBpm(127);
    EXPECT_TRUE(pQuery->match(pTrack));
    pTrack->trySetBpm(127.12);
    EXPECT_FALSE(pQuery->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString("NOT (bpm = 127.12)")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, NumericFilterAllowsSpace) {
    m_parser.setSearchColumns({"artist", "album"});
    auto pQuery(
            m_parser.parseQuery("bpm: 127.12", QString()));

    TrackPointer pTrack = newTestTrack(44100);
    pTrack->trySetBpm(127);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->trySetBpm(127.12);
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString("bpm = 127.12")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, NumericFilterOperators) {
    m_parser.setSearchColumns({"artist", "album"});
    auto pQuery(
            m_parser.parseQuery("bpm:>127.12", QString()));

    TrackPointer pTrack = newTestTrack(44100);
    pTrack->trySetBpm(127.12);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->trySetBpm(127.13);
    EXPECT_TRUE(pQuery->match(pTrack));
    EXPECT_STREQ(
        qPrintable(QString("bpm > 127.12")),
        qPrintable(pQuery->toSql()));

    pQuery = m_parser.parseQuery("bpm:>=127.12", QString());
    pTrack->trySetBpm(127.11);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->trySetBpm(127.12);
    EXPECT_TRUE(pQuery->match(pTrack));
    EXPECT_STREQ(
        qPrintable(QString("bpm >= 127.12")),
        qPrintable(pQuery->toSql()));

    pQuery = m_parser.parseQuery("bpm:<127.12", QString());
    pTrack->trySetBpm(127.12);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->trySetBpm(127.11);
    EXPECT_TRUE(pQuery->match(pTrack));
    EXPECT_STREQ(
        qPrintable(QString("bpm < 127.12")),
        qPrintable(pQuery->toSql()));

    pQuery = m_parser.parseQuery("bpm:<=127.12", QString());
    pTrack->trySetBpm(127.13);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->trySetBpm(127.12);
    EXPECT_TRUE(pQuery->match(pTrack));
    EXPECT_STREQ(
        qPrintable(QString("bpm <= 127.12")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, NumericRangeFilter) {
    m_parser.setSearchColumns({"artist", "album"});
    auto pQuery(
            m_parser.parseQuery("bpm:127.12-129", QString()));

    TrackPointer pTrack = newTestTrack(44100);
    pTrack->trySetBpm(125);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->trySetBpm(127.12);
    EXPECT_TRUE(pQuery->match(pTrack));
    pTrack->trySetBpm(129);
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
            qPrintable(QString("bpm BETWEEN 127.12 AND 129")),
            qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, MultipleFilters) {
    m_parser.setSearchColumns({"artist", "title"});
    auto pQuery(
            m_parser.parseQuery("bpm:127.12-129 artist:\"com truise\" Colorvision", QString()));

    TrackPointer pTrack = newTestTrack(44100);
    pTrack->trySetBpm(128);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setArtist("Com Truise");
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setTitle("Colorvision");
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(qPrintable(QString("(bpm BETWEEN 127.12 AND 129) AND "
                                    "((artist LIKE '%com truise%') OR "
                                    "(album_artist LIKE '%com truise%')) AND "
                                    "((artist LIKE '%colorvision%') OR (title "
                                    "LIKE '%colorvision%'))")),
            qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, ExtraFilterAppended) {
    m_parser.setSearchColumns({"artist"});
    auto pQuery(
            m_parser.parseQuery("asdf", "1 > 2"));

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
    m_parser.setSearchColumns({"artist", "album"});
    auto pQuery(
            m_parser.parseQuery("duration:1:30", QString()));

    TrackPointer pTrack = newTestTrack(44100);
    pTrack->setDuration(91);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(90);
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString("duration = 90")),
        qPrintable(pQuery->toSql()));

    pQuery = m_parser.parseQuery("duration:1m30s", QString());
    pTrack->setDuration(91);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(90);
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString("duration = 90")),
        qPrintable(pQuery->toSql()));

    pQuery = m_parser.parseQuery("duration:90", QString());
    pTrack->setDuration(91);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(90);
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
        qPrintable(QString("duration = 90")),
        qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, HumanReadableDurationSearchWithOperators) {
    m_parser.setSearchColumns({"artist", "album"});
    auto pQuery(
            m_parser.parseQuery("duration:>1:30", QString()));

    TrackPointer pTrack = newTestTrack(44100);
    pTrack->setDuration(89);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(91);
    EXPECT_TRUE(pQuery->match(pTrack));
    EXPECT_STREQ(
        qPrintable(QString("duration > 90")),
        qPrintable(pQuery->toSql()));

    pQuery = m_parser.parseQuery("duration:>=90", QString());
    pTrack->setDuration(89);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(90);
    EXPECT_TRUE(pQuery->match(pTrack));
    EXPECT_STREQ(
        qPrintable(QString("duration >= 90")),
        qPrintable(pQuery->toSql()));

    pQuery = m_parser.parseQuery("duration:>=1:30", QString());
    pTrack->setDuration(89);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(90);
    EXPECT_TRUE(pQuery->match(pTrack));
    EXPECT_STREQ(
        qPrintable(QString("duration >= 90")),
        qPrintable(pQuery->toSql()));

    pQuery = m_parser.parseQuery("duration:<2:30", QString());
    pTrack->setDuration(151);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(89);
    EXPECT_TRUE(pQuery->match(pTrack));
    EXPECT_STREQ(
        qPrintable(QString("duration < 150")),
        qPrintable(pQuery->toSql()));

    pQuery = m_parser.parseQuery("duration:<=2:30", QString());
    pTrack->setDuration(191);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(150);
    EXPECT_TRUE(pQuery->match(pTrack));
    EXPECT_STREQ(
        qPrintable(QString("duration <= 150")),
        qPrintable(pQuery->toSql()));

    pQuery = m_parser.parseQuery("duration:<=150", QString());
    pTrack->setDuration(191);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(150);
    EXPECT_TRUE(pQuery->match(pTrack));
    EXPECT_STREQ(
        qPrintable(QString("duration <= 150")),
        qPrintable(pQuery->toSql()));

    pQuery = m_parser.parseQuery("duration:<=2m30s", QString());
    pTrack->setDuration(191);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(150);
    EXPECT_TRUE(pQuery->match(pTrack));
    EXPECT_STREQ(
        qPrintable(QString("duration <= 150")),
        qPrintable(pQuery->toSql()));

    pQuery = m_parser.parseQuery("duration:<=2m", QString());
    pTrack->setDuration(191);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(110);
    EXPECT_TRUE(pQuery->match(pTrack));
    EXPECT_STREQ(
        qPrintable(QString("duration <= 120")),
        qPrintable(pQuery->toSql()));

    pQuery = m_parser.parseQuery("duration:<=2:", QString());
    pTrack->setDuration(191);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(110);
    EXPECT_TRUE(pQuery->match(pTrack));
    EXPECT_STREQ(
        qPrintable(QString("duration <= 120")),
        qPrintable(pQuery->toSql()));

    pQuery = m_parser.parseQuery("duration:>=1:3", QString());
    pTrack->setDuration(60);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(150);
    EXPECT_TRUE(pQuery->match(pTrack));
    EXPECT_STREQ(
        qPrintable(QString("duration >= 63")),
        qPrintable(pQuery->toSql()));

    // Seconds out of range
    pQuery = m_parser.parseQuery("duration:>=1:60", QString());
    pTrack->setDuration(60);
    EXPECT_FALSE(pQuery->match(pTrack));
    EXPECT_TRUE(pQuery->toSql().isEmpty());
}

TEST_F(SearchQueryParserTest, HumanReadableDurationSearchwithRangeFilter) {
    m_parser.setSearchColumns({"artist", "album"});
    auto pQuery(
            m_parser.parseQuery("duration:2:30-3:20", QString()));

    TrackPointer pTrack = newTestTrack(44100);
    pTrack->setDuration(80);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(150);
    EXPECT_TRUE(pQuery->match(pTrack));
    pTrack->setDuration(199);
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
            qPrintable(QString("duration BETWEEN 150 AND 200")),
            qPrintable(pQuery->toSql()));

    pQuery = m_parser.parseQuery("duration:2:30-200", QString());
    pTrack->setDuration(80);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(150);
    EXPECT_TRUE(pQuery->match(pTrack));
    pTrack->setDuration(199);
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
            qPrintable(QString("duration BETWEEN 150 AND 200")),
            qPrintable(pQuery->toSql()));

    pQuery = m_parser.parseQuery("duration:150-200", QString());
    pTrack->setDuration(80);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(150);
    EXPECT_TRUE(pQuery->match(pTrack));
    pTrack->setDuration(199);
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
            qPrintable(QString("duration BETWEEN 150 AND 200")),
            qPrintable(pQuery->toSql()));

    pQuery = m_parser.parseQuery("duration:2m30s-3m20s", QString());
    pTrack->setDuration(80);
    EXPECT_FALSE(pQuery->match(pTrack));
    pTrack->setDuration(150);
    EXPECT_TRUE(pQuery->match(pTrack));
    pTrack->setDuration(199);
    EXPECT_TRUE(pQuery->match(pTrack));

    EXPECT_STREQ(
            qPrintable(QString("duration BETWEEN 150 AND 200")),
            qPrintable(pQuery->toSql()));
}

TEST_F(SearchQueryParserTest, CrateFilter) {
    // User's search term
    QString searchTerm = "test";

    // Parse the user query
    auto pQuery(m_parser.parseQuery(QString("crate: %1").arg(searchTerm), QString()));

    // locations for test tracks
    const QString kTrackALocationTest(getTestDir().filePath(
            QStringLiteral("id3-test-data/cover-test-jpg.mp3")));
    const QString kTrackBLocationTest(getTestDir().filePath(
            QStringLiteral("id3-test-data/cover-test-png.mp3")));

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
    m_parser.setSearchColumns({"crate", "artist", "comment"});
    // Parse the user query
    auto pQuery(m_parser.parseQuery(QString("%1").arg(searchTerm), QString()));

    // locations for test tracks
    const QString kTrackALocationTest(getTestDir().filePath(
            QStringLiteral("id3-test-data/cover-test-jpg.mp3")));
    const QString kTrackBLocationTest(getTestDir().filePath(
            QStringLiteral("id3-test-data/cover-test-png.mp3")));
    const QString kTrackCLocationTest(
            getTestDir().filePath(QStringLiteral("id3-test-data/artist.mp3")));

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
    auto pQuery(m_parser.parseQuery(QString("crate: "), QString()));

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
    auto pQuery(m_parser.parseQuery(QString("crate: \"%1\"").arg(searchTerm), QString()));

    // locations for test tracks
    const QString kTrackALocationTest(getTestDir().filePath(
            QStringLiteral("id3-test-data/cover-test-jpg.mp3")));
    const QString kTrackBLocationTest(getTestDir().filePath(
            QStringLiteral("id3-test-data/cover-test-png.mp3")));

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
    auto pQuery(m_parser.parseQuery(QString("crate: %1 artist: asdf").arg(searchTerm), QString()));

    // locations for test tracks
    const QString kTrackALocationTest(getTestDir().filePath(
            QStringLiteral("id3-test-data/cover-test-jpg.mp3")));
    const QString kTrackBLocationTest(getTestDir().filePath(
            QStringLiteral("id3-test-data/cover-test-png.mp3")));

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
    QString searchTermA = "testA'1"; // Also a test if "'" is escaped #9419
    QString searchTermAEsc = "testA''1";
    QString searchTermB = "testB";

    // Parse the user query
    auto pQueryA(m_parser.parseQuery(
            QString("crate: %1 crate: %2").arg(searchTermA, searchTermB),
            QString()));

    // locations for test tracks
    const QString kTrackALocationTest(getTestDir().filePath(
            QStringLiteral("id3-test-data/cover-test-jpg.mp3")));
    const QString kTrackBLocationTest(getTestDir().filePath(
            QStringLiteral("id3-test-data/cover-test-png.mp3")));

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
    auto pQueryB(m_parser.parseQuery(
            QString("crate: %1 -crate: %2").arg(searchTermA, searchTermB),
            QString()));

    EXPECT_FALSE(pQueryB->match(pTrackA));
    EXPECT_TRUE(pQueryB->match(pTrackB));

    EXPECT_STREQ(
                 qPrintable("(" + m_crateFilterQuery.arg(searchTermAEsc) +
                            ") AND (NOT (" + m_crateFilterQuery.arg(searchTermB) + "))"),
                 qPrintable(pQueryB->toSql()));
}

TEST_F(SearchQueryParserTest, SplitQueryIntoWords) {
    QStringList rv = SearchQueryParser::splitQueryIntoWords(QString("a test b"));
    QStringList ex = QStringList() << "a"
                                   << "test"
                                   << "b";
    qDebug() << rv << ex;
    EXPECT_EQ(rv, ex);

    QStringList rv2 = SearchQueryParser::splitQueryIntoWords(QString("a \"test ' b\" x"));
    QStringList ex2 = QStringList() << "a"
                                    << "\"test ' b\""
                                    << "x";
    qDebug() << rv2 << ex2;
    EXPECT_EQ(rv2, ex2);

    QStringList rv3 = SearchQueryParser::splitQueryIntoWords(QString("a x"));
    QStringList ex3 = QStringList() << "a"
                                    << "x";
    qDebug() << rv3 << ex3;
    EXPECT_EQ(rv3, ex3);

    QStringList rv4 = SearchQueryParser::splitQueryIntoWords(
            QString("a crate:x title:\"S p A C e\" ~key:2m"));
    QStringList ex4 = QStringList() << "a"
                                    << "crate:x"
                                    << "title:\"S p A C e\""
                                    << "~key:2m";
    qDebug() << rv4 << ex4;
    EXPECT_EQ(rv4, ex4);
}

TEST_F(SearchQueryParserTest, QueryIsLessSpecific) {
    // Generate a file name for the temporary file
    EXPECT_TRUE(SearchQueryParser::queryIsLessSpecific(
            QStringLiteral("searchme"),
            QStringLiteral("searchm")));

    EXPECT_TRUE(SearchQueryParser::queryIsLessSpecific(
            QStringLiteral("A B C"),
            QStringLiteral("A C")));

    EXPECT_FALSE(SearchQueryParser::queryIsLessSpecific(
            QStringLiteral("A B C"),
            QStringLiteral("A D C")));

    EXPECT_TRUE(SearchQueryParser::queryIsLessSpecific(
            QStringLiteral("A D C"),
            QStringLiteral("A D C ")));

    EXPECT_TRUE(SearchQueryParser::queryIsLessSpecific(
            QStringLiteral("A D  C "),
            QStringLiteral("A D C")));

    EXPECT_FALSE(SearchQueryParser::queryIsLessSpecific(
            QStringLiteral("A B  C"),
            QStringLiteral("A D C")));

    EXPECT_TRUE(SearchQueryParser::queryIsLessSpecific(
            QStringLiteral("A D C"),
            QStringLiteral("A D C ")));

    EXPECT_TRUE(SearchQueryParser::queryIsLessSpecific(
            QStringLiteral("A D  C "),
            QStringLiteral("A D C")));

    EXPECT_FALSE(SearchQueryParser::queryIsLessSpecific(
            QStringLiteral("A B  C"),
            QStringLiteral("A D C")));

    EXPECT_TRUE(SearchQueryParser::queryIsLessSpecific(
            QStringLiteral("Abba1 Abba2 Abb"),
            QStringLiteral("Abba1 Abba Abb")));

    EXPECT_FALSE(SearchQueryParser::queryIsLessSpecific(
            QStringLiteral("Abba1 Abba2 Abb"),
            QStringLiteral("Abba1 Aba Abb")));

    EXPECT_TRUE(SearchQueryParser::queryIsLessSpecific(
            QStringLiteral("Abba1"),
            QLatin1String("")));

    EXPECT_TRUE(SearchQueryParser::queryIsLessSpecific(
            QStringLiteral("Abba1"),
            QStringLiteral("bba")));

    EXPECT_TRUE(SearchQueryParser::queryIsLessSpecific(
            QStringLiteral("crate:abc"),
            QStringLiteral("crate:ab")));

    EXPECT_FALSE(SearchQueryParser::queryIsLessSpecific(
            QStringLiteral("crate:\"a b c\""),
            QStringLiteral("crate:\"a c\"")));

    EXPECT_FALSE(SearchQueryParser::queryIsLessSpecific(
            QStringLiteral("-crate:\"a b c\""),
            QStringLiteral("crate:\"a b c\"")));

    EXPECT_FALSE(SearchQueryParser::queryIsLessSpecific(
            QStringLiteral("-crate:\"a b c\""),
            QStringLiteral("crate:\"a b c\"")));
}
