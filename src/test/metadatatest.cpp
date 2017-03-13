#include <gtest/gtest.h>

#include "metadata/trackmetadatataglib.h"
#include "util/math.h"

#include <QtDebug>

namespace {

class MetadataTest : public testing::Test {
  protected:

    MetadataTest() {
    }

    virtual void SetUp() {
    }

    virtual void TearDown() {
    }

    double parseBpm(QString inputValue, bool expectedResult, double expectedValue) {
        //qDebug() << "parseBpm" << inputValue << expectedResult << expectedValue;

        bool actualResult;
        const double actualValue = Mixxx::TrackMetadata::parseBpm(inputValue, &actualResult);

        EXPECT_EQ(expectedResult, actualResult);
        EXPECT_DOUBLE_EQ(expectedValue, actualValue);

//        if (actualResult) {
//            qDebug() << "BPM:" << inputValue << "->" << Mixxx::TrackMetadata::formatBpm(actualValue);
//        }

        return actualResult;
    }

    double parseReplayGain(QString inputValue, bool expectedResult, float expectedValue) {
        //qDebug() << "parseReplayGain" << inputValue << expectedResult << expectedValue;

        bool actualResult;
        const double actualValue = Mixxx::TrackMetadata::parseReplayGain(inputValue, &actualResult);

        EXPECT_EQ(expectedResult, actualResult);
        EXPECT_FLOAT_EQ(expectedValue, actualValue);

//        if (actualResult) {
//            qDebug() << "ReplayGain:" << inputValue << "->" << Mixxx::TrackMetadata::formatReplayGain(actualValue);
//        }

        return actualResult;
    }
};

TEST_F(MetadataTest, ParseBpmPrecision) {
    parseBpm("128.1234", true, 128.1234); // 4 fractional digits
}

TEST_F(MetadataTest, ParseBpmValidRange) {
    for (int bpm100 = int(Mixxx::TrackMetadata::kBpmMin) * 100; int(Mixxx::TrackMetadata::kBpmMax) * 100 >= bpm100; ++bpm100) {
        const double expectedValue = bpm100 / 100.0;
        const QString inputValues[] = {
                QString("%1").arg(expectedValue),
                QString("  %1 ").arg(expectedValue),
        };
        for (size_t i = 0; i < sizeof(inputValues) / sizeof(inputValues[0]); ++i) {
            parseBpm(inputValues[i], true, expectedValue);
        }
    }
}

TEST_F(MetadataTest, ParseBpmDecimalScaling) {
    parseBpm("345678", true, 34.5678);
    parseBpm("2345678", true, 234.5678);
}

TEST_F(MetadataTest, ParseBpmInvalid) {
    parseBpm("", false, Mixxx::TrackMetadata::kBpmUndefined);
    parseBpm("abcde", false, Mixxx::TrackMetadata::kBpmUndefined);
    parseBpm("0 dBA", false, Mixxx::TrackMetadata::kBpmUndefined);
}

TEST_F(MetadataTest, ParseReplayGainDbValidRange) {
    for (int replayGainDb = -100; 100 >= replayGainDb; ++replayGainDb) {
        const QString inputValues[] = {
                QString("%1 ").arg(replayGainDb),
                QString("  %1dB ").arg(replayGainDb),
                QString("  %1 DB ").arg(replayGainDb),
                QString("  %1db ").arg(replayGainDb)
        };
        float expectedValue;
        if (0 != replayGainDb) {
            // regular case
            expectedValue = db2ratio(double(replayGainDb));
        } else {
            // special case: 0 dB -> undefined
            expectedValue = Mixxx::TrackMetadata::kReplayGainUndefined;
        }
        for (size_t i = 0; i < sizeof(inputValues) / sizeof(inputValues[0]); ++i) {
            parseReplayGain(inputValues[i], true, expectedValue);
            if (0 <= replayGainDb) {
                parseReplayGain(QString("  + ") + inputValues[i], true, expectedValue);
            }
        }
    }
}

TEST_F(MetadataTest, ParseReplayGainDbInvalid) {
    parseReplayGain("", false, Mixxx::TrackMetadata::kReplayGainUndefined);
    parseReplayGain("abcde", false, Mixxx::TrackMetadata::kReplayGainUndefined);
    parseReplayGain("0 dBA", false, Mixxx::TrackMetadata::kReplayGainUndefined);
}

TEST_F(MetadataTest, ID3v2Year) {
    const char* kYears[] = {
            " 1987  ",
            " 2001-01-01",
            "1997-12", // yyyy-MM
            "1977-W43", // year + week
            "2002 -12 - 31 ",
            "2015 -02 - 04T 18:43",
            "2015 -02 - 04  18:43",
            "2015 -02 - 04  18:43 followed by arbitrary text"
    };
    // Only ID3v2.3.0 and ID3v2.4.0 are supported for writing
    for (int majorVersion = 3; 4 >= majorVersion; ++majorVersion) {
        qDebug() << "majorVersion" << majorVersion;
        for (size_t i = 0; i < sizeof(kYears) / sizeof(kYears[0]); ++i) {
            const QString year(kYears[i]);
            TagLib::ID3v2::Tag tag;
            tag.header()->setMajorVersion(majorVersion);
            {
                Mixxx::TrackMetadata trackMetadata;
                trackMetadata.setYear(year);
                writeTrackMetadataIntoID3v2Tag(&tag, trackMetadata);
            }
            Mixxx::TrackMetadata trackMetadata;
            readTrackMetadataFromID3v2Tag(&trackMetadata, tag);
            if (4 > majorVersion) {
                // ID3v2.3.0: parsed + formatted
                const QString actualYear(trackMetadata.getYear());
                const QDate expectedDate(Mixxx::TrackMetadata::parseDate(year));
                if (expectedDate.isValid()) {
                    // Only the date part can be stored in an ID3v2.3.0 tag
                    EXPECT_EQ(Mixxx::TrackMetadata::formatDate(expectedDate), actualYear);
                } else {
                    // numeric year (without month/day)
                    EXPECT_EQ(Mixxx::TrackMetadata::reformatYear(year), actualYear);
                }
            } else {
                // ID3v2.4.0: currently unverified/unmodified
                EXPECT_EQ(year, trackMetadata.getYear());
            }
        }
    }
}

TEST_F(MetadataTest, CalendarYear) {
    // Parsing
    EXPECT_EQ(2014, Mixxx::TrackMetadata::parseCalendarYear("2014-04-29T07:00:00Z"));
    EXPECT_EQ(2014, Mixxx::TrackMetadata::parseCalendarYear("2014-04-29"));
    EXPECT_EQ(2014, Mixxx::TrackMetadata::parseCalendarYear("2014"));
    EXPECT_EQ(2015, Mixxx::TrackMetadata::parseCalendarYear("2015-02"));
    EXPECT_EQ(1997, Mixxx::TrackMetadata::parseCalendarYear("1997-W43"));
    EXPECT_EQ(1, Mixxx::TrackMetadata::parseCalendarYear("1"));
    EXPECT_EQ(Mixxx::TrackMetadata::kCalendarYearInvalid, Mixxx::TrackMetadata::parseCalendarYear("0"));
    EXPECT_EQ(Mixxx::TrackMetadata::kCalendarYearInvalid, Mixxx::TrackMetadata::parseCalendarYear("-1"));
    EXPECT_EQ(Mixxx::TrackMetadata::kCalendarYearInvalid, Mixxx::TrackMetadata::parseCalendarYear("year"));

    // Formatting
    EXPECT_EQ("2014", Mixxx::TrackMetadata::formatCalendarYear("2014-04-29T07:00:00Z"));
    EXPECT_EQ("2014", Mixxx::TrackMetadata::formatCalendarYear("2014-04-29"));
    EXPECT_EQ("2014", Mixxx::TrackMetadata::formatCalendarYear("2014"));
    EXPECT_EQ("2015", Mixxx::TrackMetadata::formatCalendarYear("2015-02"));
    EXPECT_EQ("1997", Mixxx::TrackMetadata::formatCalendarYear("1997-W43"));
    EXPECT_EQ("", Mixxx::TrackMetadata::formatCalendarYear("0"));
    EXPECT_EQ("", Mixxx::TrackMetadata::formatCalendarYear("-1"));
    EXPECT_EQ("", Mixxx::TrackMetadata::formatCalendarYear("year"));
}

}  // namespace
