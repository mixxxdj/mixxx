#include <gtest/gtest.h>

#include "metadata/trackmetadata.h"
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
    for (int bpm100 = int(Mixxx::TrackMetadata::BPM_MIN) * 100; int(Mixxx::TrackMetadata::BPM_MAX) * 100 >= bpm100; ++bpm100) {
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
    parseBpm("", false, Mixxx::TrackMetadata::BPM_UNDEFINED);
    parseBpm("abcde", false, Mixxx::TrackMetadata::BPM_UNDEFINED);
    parseBpm("0 dBA", false, Mixxx::TrackMetadata::BPM_UNDEFINED);
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
            expectedValue = Mixxx::TrackMetadata::REPLAYGAIN_UNDEFINED;
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
    parseReplayGain("", false, Mixxx::TrackMetadata::REPLAYGAIN_UNDEFINED);
    parseReplayGain("abcde", false, Mixxx::TrackMetadata::REPLAYGAIN_UNDEFINED);
    parseReplayGain("0 dBA", false, Mixxx::TrackMetadata::REPLAYGAIN_UNDEFINED);
}

}  // namespace
