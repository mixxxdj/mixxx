#include <gtest/gtest.h>

#include "util/replaygain.h"

#include <QtDebug>

namespace {

class ReplayGainTest : public testing::Test {
  protected:

    ReplayGainTest() {
    }

    virtual void SetUp() {
    }

    virtual void TearDown() {
    }

    double parseGain2Ratio(QString inputValue, bool expectedResult, float expectedValue) {
        //qDebug() << "parseGain2Ratio" << inputValue << expectedResult << expectedValue;

        bool actualResult;
        const double actualValue = Mixxx::ReplayGain::parseGain2Ratio(inputValue, &actualResult);

        EXPECT_EQ(expectedResult, actualResult);
        EXPECT_FLOAT_EQ(expectedValue, actualValue);

        return actualResult;
    }

    void normalizeRatio(double expectedResult) {
        const double actualResult = Mixxx::ReplayGain::normalizeRatio(expectedResult);
        EXPECT_EQ(expectedResult, actualResult);
    }
};

TEST_F(ReplayGainTest, ParseReplayGainDbValidRange) {
    for (int replayGainDb = -100; 100 >= replayGainDb; ++replayGainDb) {
        const QString inputValues[] = {
                QString("%1 ").arg(replayGainDb),
                QString("  %1dB ").arg(replayGainDb),
                QString("  %1 DB ").arg(replayGainDb),
                QString("  %1db ").arg(replayGainDb)
        };
        float expectedValue;
        expectedValue = db2ratio(double(replayGainDb));
        for (size_t i = 0; i < sizeof(inputValues) / sizeof(inputValues[0]); ++i) {
            parseGain2Ratio(inputValues[i], true, expectedValue);
            if (0 <= replayGainDb) {
                parseGain2Ratio(QString("  + ") + inputValues[i], true, expectedValue);
            }
        }
    }
}

TEST_F(ReplayGainTest, ParseReplayGainDbInvalid) {
    parseGain2Ratio("", false, Mixxx::ReplayGain::kRatioUndefined);
    parseGain2Ratio("abcde", false, Mixxx::ReplayGain::kRatioUndefined);
    parseGain2Ratio("0 dBA", false, Mixxx::ReplayGain::kRatioUndefined);
}

TEST_F(ReplayGainTest, NormalizeReplayGain) {
    normalizeRatio(Mixxx::ReplayGain::kRatioUndefined);
    normalizeRatio(Mixxx::ReplayGain::kRatioMin);
    normalizeRatio(-Mixxx::ReplayGain::kRatioMin);
    normalizeRatio(Mixxx::ReplayGain::kRatio0dB);
    normalizeRatio(-Mixxx::ReplayGain::kRatio0dB);
}

} // anonymous namespace
