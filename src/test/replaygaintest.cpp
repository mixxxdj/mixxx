#include <gtest/gtest.h>

#include "track/replaygain.h"

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

    double ratioFromString(QString inputValue, bool expectedResult, double expectedValue) {
        //qDebug() << "ratioFromString" << inputValue << expectedResult << expectedValue;

        bool actualResult;
        const double actualValue = Mixxx::ReplayGain::ratioFromString(inputValue, &actualResult);

        EXPECT_EQ(expectedResult, actualResult);
        EXPECT_FLOAT_EQ(expectedValue, actualValue);

        return actualResult;
    }

    void normalizeRatio(double expectedResult) {
        const double actualResult = Mixxx::ReplayGain::normalizeRatio(expectedResult);
        if (Mixxx::ReplayGain::isValidRatio(expectedResult)) {
            EXPECT_EQ(expectedResult, actualResult);
        } else {
            EXPECT_EQ(Mixxx::ReplayGain::kRatioUndefined, actualResult);
        }
    }

    CSAMPLE peakFromString(QString inputValue, bool expectedResult, CSAMPLE expectedValue) {
        //qDebug() << "peakFromString" << inputValue << expectedResult << expectedValue;

        bool actualResult;
        const CSAMPLE actualValue = Mixxx::ReplayGain::peakFromString(inputValue, &actualResult);

        EXPECT_EQ(expectedResult, actualResult);
        EXPECT_FLOAT_EQ(expectedValue, actualValue);

        return actualResult;
    }

    void normalizePeak(CSAMPLE expectedResult) {
        const CSAMPLE actualResult = Mixxx::ReplayGain::normalizePeak(expectedResult);
        if (Mixxx::ReplayGain::isValidPeak(expectedResult)) {
            EXPECT_EQ(expectedResult, actualResult);
        } else {
            EXPECT_EQ(Mixxx::ReplayGain::kPeakUndefined, actualResult);
        }
    }
};

TEST_F(ReplayGainTest, RatioFromString0dB) {
    ratioFromString("0 dB", true, Mixxx::ReplayGain::kRatio0dB);
    ratioFromString("0.0dB", true, Mixxx::ReplayGain::kRatio0dB);
    ratioFromString("0 DB", true, Mixxx::ReplayGain::kRatio0dB);
    ratioFromString("-0 Db", true, Mixxx::ReplayGain::kRatio0dB);
    ratioFromString("+0db", true, Mixxx::ReplayGain::kRatio0dB);
}

TEST_F(ReplayGainTest, RatioFromStringValidRange) {
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
            ratioFromString(inputValues[i], true, expectedValue);
            if (0 <= replayGainDb) {
                ratioFromString(QString("  + ") + inputValues[i], true, expectedValue);
            }
        }
    }
}

TEST_F(ReplayGainTest, RatioFromStringInvalid) {
    ratioFromString("", false, Mixxx::ReplayGain::kRatioUndefined);
    ratioFromString("abcde", false, Mixxx::ReplayGain::kRatioUndefined);
    ratioFromString("0 dBA", false, Mixxx::ReplayGain::kRatioUndefined);
    ratioFromString("--2 dB", false, Mixxx::ReplayGain::kRatioUndefined);
    ratioFromString("+-2 dB", false, Mixxx::ReplayGain::kRatioUndefined);
    ratioFromString("-+2 dB", false, Mixxx::ReplayGain::kRatioUndefined);
    ratioFromString("++2 dB", false, Mixxx::ReplayGain::kRatioUndefined);
}

TEST_F(ReplayGainTest, NormalizeRatio) {
    normalizeRatio(Mixxx::ReplayGain::kRatioUndefined);
    normalizeRatio(Mixxx::ReplayGain::kRatioMin);
    normalizeRatio(-Mixxx::ReplayGain::kRatioMin);
    normalizeRatio(Mixxx::ReplayGain::kRatio0dB);
    normalizeRatio(-Mixxx::ReplayGain::kRatio0dB);
}

TEST_F(ReplayGainTest, PeakFromStringValid) {
    peakFromString("0", true, Mixxx::ReplayGain::kPeakMin);
    peakFromString("+0", true, Mixxx::ReplayGain::kPeakMin);
    peakFromString("-0", true, Mixxx::ReplayGain::kPeakMin);
    peakFromString("0.0", true, Mixxx::ReplayGain::kPeakMin);
    peakFromString("+0.0", true, Mixxx::ReplayGain::kPeakMin);
    peakFromString("-0.0", true, Mixxx::ReplayGain::kPeakMin);
    peakFromString("1", true, Mixxx::ReplayGain::kPeakClip);
    peakFromString("+1", true, Mixxx::ReplayGain::kPeakClip);
    peakFromString("1.0", true, Mixxx::ReplayGain::kPeakClip);
    peakFromString("+1.0", true, Mixxx::ReplayGain::kPeakClip);
    peakFromString("  0.12345  ", true, 0.12345);
    peakFromString("  1.2345", true, 1.2345);
}

TEST_F(ReplayGainTest, PeakFromStringInvalid) {
    peakFromString("", false, Mixxx::ReplayGain::kPeakUndefined);
    peakFromString("-1", false, Mixxx::ReplayGain::kPeakUndefined);
    peakFromString("-0.12345", false, Mixxx::ReplayGain::kPeakUndefined);
    peakFromString("--1.0", false, Mixxx::ReplayGain::kPeakUndefined);
    peakFromString("+-1.0", false, Mixxx::ReplayGain::kPeakUndefined);
    peakFromString("-+1.0", false, Mixxx::ReplayGain::kPeakUndefined);
    peakFromString("++1.0", false, Mixxx::ReplayGain::kPeakUndefined);
    peakFromString("+abcde", false, Mixxx::ReplayGain::kPeakUndefined);
}

TEST_F(ReplayGainTest, NormalizePeak) {
    normalizePeak(Mixxx::ReplayGain::kPeakUndefined);
    normalizePeak(Mixxx::ReplayGain::kPeakMin);
    normalizePeak(-Mixxx::ReplayGain::kPeakMin);
    normalizePeak(Mixxx::ReplayGain::kPeakClip);
    normalizePeak(-Mixxx::ReplayGain::kPeakClip);
    normalizePeak(Mixxx::ReplayGain::kPeakClip + Mixxx::ReplayGain::kPeakClip);
}

} // anonymous namespace
