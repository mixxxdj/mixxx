#include <gtest/gtest.h>

#include "track/trackmetadatataglib.h"

#include <QtDebug>

namespace {

const double kBpmValueMax = 300.0;

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
        const double actualValue = mixxx::Bpm::valueFromString(inputValue, &actualResult);

        EXPECT_EQ(expectedResult, actualResult);
        EXPECT_DOUBLE_EQ(expectedValue, actualValue);

//        if (actualResult) {
//            qDebug() << "BPM:" << inputValue << "->" << mixxx::Bpm::valueToString(actualValue);
//        }

        return actualResult;
    }

    void normalizeBpm(double normalizedValue) {
        mixxx::Bpm normalizedBpm(normalizedValue);
        normalizedBpm.normalizeValue(); // re-normalize
        // Expected: Re-normalization does not change the value
        // that should already be normalized.
        EXPECT_EQ(normalizedBpm.getValue(), normalizedValue);
    }
};

TEST_F(MetadataTest, ParseBpmPrecision) {
    parseBpm("128.1234", true, 128.1234); // 4 fractional digits
}

TEST_F(MetadataTest, ParseBpmValidRange) {
    for (int bpm100 = int(mixxx::Bpm::kValueMin) * 100; kBpmValueMax * 100 >= bpm100; ++bpm100) {
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

TEST_F(MetadataTest, ParseBpmInvalid) {
    parseBpm("", false, mixxx::Bpm::kValueUndefined);
    parseBpm("abcde", false, mixxx::Bpm::kValueUndefined);
    parseBpm("0 dBA", false, mixxx::Bpm::kValueUndefined);
}

TEST_F(MetadataTest, NormalizeBpm) {
    normalizeBpm(mixxx::Bpm::kValueUndefined);
    normalizeBpm(mixxx::Bpm::kValueMin);
    normalizeBpm(mixxx::Bpm::kValueMin - 1.0);
    normalizeBpm(mixxx::Bpm::kValueMin + 1.0);
    normalizeBpm(-mixxx::Bpm::kValueMin);
    normalizeBpm(kBpmValueMax);
    normalizeBpm(kBpmValueMax - 1.0);
    normalizeBpm(kBpmValueMax + 1.0);
    normalizeBpm(-kBpmValueMax);
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
                mixxx::TrackMetadata trackMetadata;
                trackMetadata.setYear(year);
                writeTrackMetadataIntoID3v2Tag(&tag, trackMetadata);
            }
            mixxx::TrackMetadata trackMetadata;
            readTrackMetadataFromID3v2Tag(&trackMetadata, tag);
            if (4 > majorVersion) {
                // ID3v2.3.0: parsed + formatted
                const QString actualYear(trackMetadata.getYear());
                const QDate expectedDate(mixxx::TrackMetadata::parseDate(year));
                if (expectedDate.isValid()) {
                    // Only the date part can be stored in an ID3v2.3.0 tag
                    EXPECT_EQ(mixxx::TrackMetadata::formatDate(expectedDate), actualYear);
                } else {
                    // numeric year (without month/day)
                    EXPECT_EQ(mixxx::TrackMetadata::reformatYear(year), actualYear);
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
    EXPECT_EQ(2014, mixxx::TrackMetadata::parseCalendarYear("2014-04-29T07:00:00Z"));
    EXPECT_EQ(2014, mixxx::TrackMetadata::parseCalendarYear("2014-04-29"));
    EXPECT_EQ(2014, mixxx::TrackMetadata::parseCalendarYear("2014"));
    EXPECT_EQ(2015, mixxx::TrackMetadata::parseCalendarYear("2015-02"));
    EXPECT_EQ(1997, mixxx::TrackMetadata::parseCalendarYear("1997-W43"));
    EXPECT_EQ(1, mixxx::TrackMetadata::parseCalendarYear("1"));
    EXPECT_EQ(mixxx::TrackMetadata::kCalendarYearInvalid, mixxx::TrackMetadata::parseCalendarYear("0"));
    EXPECT_EQ(mixxx::TrackMetadata::kCalendarYearInvalid, mixxx::TrackMetadata::parseCalendarYear("-1"));
    EXPECT_EQ(mixxx::TrackMetadata::kCalendarYearInvalid, mixxx::TrackMetadata::parseCalendarYear("year"));

    // Formatting
    EXPECT_EQ("2014", mixxx::TrackMetadata::formatCalendarYear("2014-04-29T07:00:00Z"));
    EXPECT_EQ("2014", mixxx::TrackMetadata::formatCalendarYear("2014-04-29"));
    EXPECT_EQ("2014", mixxx::TrackMetadata::formatCalendarYear("2014"));
    EXPECT_EQ("2015", mixxx::TrackMetadata::formatCalendarYear("2015-02"));
    EXPECT_EQ("1997", mixxx::TrackMetadata::formatCalendarYear("1997-W43"));
    EXPECT_EQ("", mixxx::TrackMetadata::formatCalendarYear("0"));
    EXPECT_EQ("", mixxx::TrackMetadata::formatCalendarYear("-1"));
    EXPECT_EQ("", mixxx::TrackMetadata::formatCalendarYear("year"));
}

}  // namespace
