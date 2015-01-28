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

    double parseBpm(double initialValue, QString inputValue, bool expectedResult, double expectedValue) {
        //qDebug() << "parseBpm" << initialValue << inputValue << expectedResult << expectedValue;

        Mixxx::TrackMetadata trackMetadata;
        trackMetadata.setBpm(initialValue);

        const bool actualResult = trackMetadata.setBpmString(inputValue);

        EXPECT_EQ(expectedResult, actualResult);
        EXPECT_DOUBLE_EQ(trackMetadata.getBpm(), expectedValue);

        return actualResult;
    }

    float parseReplayGainDb(float initialValue, QString inputValue, bool expectedResult, float expectedValue) {
        //qDebug() << "parseReplayGainDb" << initialValue << inputValue << expectedResult << expectedValue;

        Mixxx::TrackMetadata trackMetadata;
        trackMetadata.setReplayGain(initialValue);

        const bool actualResult = trackMetadata.setReplayGainDbString(inputValue);

        EXPECT_EQ(expectedResult, actualResult);
        EXPECT_FLOAT_EQ(trackMetadata.getReplayGain(), expectedValue);

        return actualResult;
    }
};

TEST_F(MetadataTest, ParseBpmPrecision) {
    parseBpm(100.0, "128.1234", true, 128.1234); // 4 fractional digits
}

TEST_F(MetadataTest, ParseBpmValidRange) {
    for (int bpm100 = int(Mixxx::TrackMetadata::BPM_MIN) * 100; int(Mixxx::TrackMetadata::BPM_MAX) * 100 >= bpm100; ++bpm100) {
        const double expectedValue = bpm100 / 100.0;
        const double initialValues[] = {
                Mixxx::TrackMetadata::BPM_UNDEFINED,
                128.5
        };
        const QString inputValues[] = {
                QString("%1").arg(expectedValue),
                QString("  %1 ").arg(expectedValue),
        };
        for (size_t i = 0; i < sizeof(initialValues) / sizeof(initialValues[0]); ++i) {
            for (size_t j = 0; j < sizeof(inputValues) / sizeof(inputValues[0]); ++j) {
                parseBpm(initialValues[i], inputValues[j], true, expectedValue);
            }
        }
    }
}

TEST_F(MetadataTest, ParseBpmDecimalScaling) {
    parseBpm(100.0, "345678", true, 34.5678);
    parseBpm(100.0, "2345678", true, 234.5678);
}

TEST_F(MetadataTest, ParseBpmInvalid) {
    parseBpm(Mixxx::TrackMetadata::BPM_UNDEFINED, "", false, Mixxx::TrackMetadata::BPM_UNDEFINED);
    parseBpm(123.45, "abcde", false, 123.45);
}

TEST_F(MetadataTest, ParseReplayGainDbValidRange) {
    for (int replayGainDb = -100; 100 >= replayGainDb; ++replayGainDb) {
        const float initialValues[] = {
                Mixxx::TrackMetadata::REPLAYGAIN_UNDEFINED,
                0.5f
        };
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
        for (size_t i = 0; i < sizeof(initialValues) / sizeof(initialValues[0]); ++i) {
            for (size_t j = 0; j < sizeof(inputValues) / sizeof(inputValues[0]); ++j) {
                parseReplayGainDb(initialValues[i], inputValues[j], true, expectedValue);
                if (0 <= replayGainDb) {
                    parseReplayGainDb(initialValues[i], QString("  + ") + inputValues[j], true, expectedValue);
                }
            }
        }
    }
}

TEST_F(MetadataTest, ParseReplayGainDbInvalid) {
    parseReplayGainDb(Mixxx::TrackMetadata::REPLAYGAIN_UNDEFINED, "", false, Mixxx::TrackMetadata::REPLAYGAIN_UNDEFINED);
    parseReplayGainDb(0.5, "abcde", false, 0.5);
}

}  // namespace
