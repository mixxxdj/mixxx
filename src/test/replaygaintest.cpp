#include <gtest/gtest.h>

#include "track/replaygain.h"

#include <QtDebug>

namespace {

class ReplayGainTest : public testing::Test {
  protected:
    double ratioFromString(const QString& inputValue, bool expectedResult, double expectedValue) {
        //qDebug() << "ratioFromString" << inputValue << expectedResult << expectedValue;

        bool actualResult;
        const double actualValue = mixxx::ReplayGain::ratioFromString(inputValue, &actualResult);

        EXPECT_EQ(expectedResult, actualResult);
        EXPECT_NEAR(expectedValue, actualValue, 0.005);

        return actualResult;
    }

    void normalizeRatio(double expectedResult) {
        const double actualResult = mixxx::ReplayGain::normalizeRatio(expectedResult);
        if (mixxx::ReplayGain::isValidRatio(expectedResult)) {
            EXPECT_EQ(expectedResult, actualResult);
        } else {
            EXPECT_EQ(mixxx::ReplayGain::kRatioUndefined, actualResult);
        }
    }

    CSAMPLE peakFromString(const QString& inputValue, bool expectedResult, CSAMPLE expectedValue) {
        //qDebug() << "peakFromString" << inputValue << expectedResult << expectedValue;

        bool actualResult;
        const CSAMPLE actualValue = mixxx::ReplayGain::peakFromString(inputValue, &actualResult);

        EXPECT_EQ(expectedResult, actualResult);
        EXPECT_FLOAT_EQ(expectedValue, actualValue);

        return actualResult;
    }

    void normalizePeak(CSAMPLE expectedResult) {
        const CSAMPLE actualResult = mixxx::ReplayGain::normalizePeak(expectedResult);
        if (mixxx::ReplayGain::isValidPeak(expectedResult)) {
            EXPECT_EQ(expectedResult, actualResult);
        } else {
            EXPECT_EQ(mixxx::ReplayGain::kPeakUndefined, actualResult);
        }
    }
};

TEST_F(ReplayGainTest, hasRatio) {
    mixxx::ReplayGain replayGain;
    EXPECT_FALSE(replayGain.hasRatio());
    replayGain.setRatio(mixxx::ReplayGain::kRatioUndefined);
    EXPECT_FALSE(replayGain.hasRatio());
    replayGain.setRatio(mixxx::ReplayGain::kRatioMin); // exclusive
    EXPECT_FALSE(replayGain.hasRatio());
    replayGain.setRatio(mixxx::ReplayGain::kRatio0dB);
    EXPECT_TRUE(replayGain.hasRatio());
    replayGain.resetRatio();
    EXPECT_FALSE(replayGain.hasRatio());
}

TEST_F(ReplayGainTest, hasPeak) {
    mixxx::ReplayGain replayGain;
    EXPECT_FALSE(replayGain.hasPeak());
    replayGain.setPeak(mixxx::ReplayGain::kPeakUndefined);
    EXPECT_FALSE(replayGain.hasPeak());
    replayGain.setPeak(mixxx::ReplayGain::kPeakMin);
    EXPECT_TRUE(replayGain.hasPeak());
    replayGain.setPeak(mixxx::ReplayGain::kPeakClip);
    EXPECT_TRUE(replayGain.hasPeak());
    replayGain.resetPeak();
    EXPECT_FALSE(replayGain.hasPeak());
}

TEST_F(ReplayGainTest, RatioFromString0dB) {
    ratioFromString("0 dB", true, mixxx::ReplayGain::kRatio0dB);
    ratioFromString("0.0dB", true, mixxx::ReplayGain::kRatio0dB);
    ratioFromString("0 DB", true, mixxx::ReplayGain::kRatio0dB);
    ratioFromString("-0 Db", true, mixxx::ReplayGain::kRatio0dB);
    ratioFromString("+0db", true, mixxx::ReplayGain::kRatio0dB);
}

TEST_F(ReplayGainTest, RatioFromStringValidRange) {
    for (int replayGainDb = -100; 100 >= replayGainDb; ++replayGainDb) {
        const QString inputValues[] = {
                QString("%1 ").arg(replayGainDb),
                QString("  %1dB ").arg(replayGainDb),
                QString("  %1 DB ").arg(replayGainDb),
                QString("  %1db ").arg(replayGainDb)
        };
        const auto expectedValue = static_cast<float>(db2ratio(static_cast<double>(replayGainDb)));
        for (size_t i = 0; i < sizeof(inputValues) / sizeof(inputValues[0]); ++i) {
            ratioFromString(inputValues[i], true, expectedValue);
            if (0 <= replayGainDb) {
                ratioFromString(QString("  + ") + inputValues[i], true, expectedValue);
            }
        }
    }
}

TEST_F(ReplayGainTest, RatioFromStringInvalid) {
    ratioFromString("", false, mixxx::ReplayGain::kRatioUndefined);
    ratioFromString("abcde", false, mixxx::ReplayGain::kRatioUndefined);
    ratioFromString("0 dBA", false, mixxx::ReplayGain::kRatioUndefined);
    ratioFromString("--2 dB", false, mixxx::ReplayGain::kRatioUndefined);
    ratioFromString("+-2 dB", false, mixxx::ReplayGain::kRatioUndefined);
    ratioFromString("-+2 dB", false, mixxx::ReplayGain::kRatioUndefined);
    ratioFromString("++2 dB", false, mixxx::ReplayGain::kRatioUndefined);
}

TEST_F(ReplayGainTest, NormalizeRatio) {
    normalizeRatio(mixxx::ReplayGain::kRatioUndefined);
    normalizeRatio(mixxx::ReplayGain::kRatioMin);
    normalizeRatio(-mixxx::ReplayGain::kRatioMin);
    normalizeRatio(mixxx::ReplayGain::kRatio0dB);
    normalizeRatio(-mixxx::ReplayGain::kRatio0dB);
}

TEST_F(ReplayGainTest, PeakFromStringValid) {
    peakFromString("0", true, mixxx::ReplayGain::kPeakMin);
    peakFromString("+0", true, mixxx::ReplayGain::kPeakMin);
    peakFromString("-0", true, mixxx::ReplayGain::kPeakMin);
    peakFromString("0.0", true, mixxx::ReplayGain::kPeakMin);
    peakFromString("+0.0", true, mixxx::ReplayGain::kPeakMin);
    peakFromString("-0.0", true, mixxx::ReplayGain::kPeakMin);
    peakFromString("1", true, mixxx::ReplayGain::kPeakClip);
    peakFromString("+1", true, mixxx::ReplayGain::kPeakClip);
    peakFromString("1.0", true, mixxx::ReplayGain::kPeakClip);
    peakFromString("+1.0", true, mixxx::ReplayGain::kPeakClip);
    peakFromString("  0.12345  ", true, 0.12345f);
    peakFromString("  1.2345", true, 1.2345f);
}

TEST_F(ReplayGainTest, PeakFromStringInvalid) {
    peakFromString("", false, mixxx::ReplayGain::kPeakUndefined);
    peakFromString("-1", false, mixxx::ReplayGain::kPeakUndefined);
    peakFromString("-0.12345", false, mixxx::ReplayGain::kPeakUndefined);
    peakFromString("--1.0", false, mixxx::ReplayGain::kPeakUndefined);
    peakFromString("+-1.0", false, mixxx::ReplayGain::kPeakUndefined);
    peakFromString("-+1.0", false, mixxx::ReplayGain::kPeakUndefined);
    peakFromString("++1.0", false, mixxx::ReplayGain::kPeakUndefined);
    peakFromString("+abcde", false, mixxx::ReplayGain::kPeakUndefined);
}

TEST_F(ReplayGainTest, NormalizePeak) {
    normalizePeak(mixxx::ReplayGain::kPeakUndefined);
    normalizePeak(mixxx::ReplayGain::kPeakMin);
    normalizePeak(-mixxx::ReplayGain::kPeakMin);
    normalizePeak(mixxx::ReplayGain::kPeakClip);
    normalizePeak(-mixxx::ReplayGain::kPeakClip);
    normalizePeak(mixxx::ReplayGain::kPeakClip + mixxx::ReplayGain::kPeakClip);
}

} // anonymous namespace
