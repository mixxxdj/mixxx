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

    double parseGain2Ratio(QString inputValue, bool expectedResult, double expectedValue) {
        //qDebug() << "parseGain2Ratio" << inputValue << expectedResult << expectedValue;

        bool actualResult;
        const double actualValue = Mixxx::ReplayGain::parseGain2Ratio(inputValue, &actualResult);

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

    CSAMPLE parsePeak(QString inputValue, bool expectedResult, CSAMPLE expectedValue) {
        //qDebug() << "parsePeak" << inputValue << expectedResult << expectedValue;

        bool actualResult;
        const CSAMPLE actualValue = Mixxx::ReplayGain::parsePeak(inputValue, &actualResult);

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

TEST_F(ReplayGainTest, ParseGain2RatioValidRange) {
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

TEST_F(ReplayGainTest, ParseGain2RatioInvalid) {
    parseGain2Ratio("", false, Mixxx::ReplayGain::kRatioUndefined);
    parseGain2Ratio("abcde", false, Mixxx::ReplayGain::kRatioUndefined);
    parseGain2Ratio("0 dBA", false, Mixxx::ReplayGain::kRatioUndefined);
    parseGain2Ratio("--2 dB", false, Mixxx::ReplayGain::kRatioUndefined);
    parseGain2Ratio("+-2 dB", false, Mixxx::ReplayGain::kRatioUndefined);
    parseGain2Ratio("-+2 dB", false, Mixxx::ReplayGain::kRatioUndefined);
    parseGain2Ratio("++2 dB", false, Mixxx::ReplayGain::kRatioUndefined);
}

TEST_F(ReplayGainTest, NormalizeRatio) {
    normalizeRatio(Mixxx::ReplayGain::kRatioUndefined);
    normalizeRatio(Mixxx::ReplayGain::kRatioMin);
    normalizeRatio(-Mixxx::ReplayGain::kRatioMin);
    normalizeRatio(Mixxx::ReplayGain::kRatio0dB);
    normalizeRatio(-Mixxx::ReplayGain::kRatio0dB);
}

TEST_F(ReplayGainTest, ParsePeakValid) {
    parsePeak("0", true, Mixxx::ReplayGain::kPeakMin);
    parsePeak("+0", true, Mixxx::ReplayGain::kPeakMin);
    parsePeak("-0", true, Mixxx::ReplayGain::kPeakMin);
    parsePeak("0.0", true, Mixxx::ReplayGain::kPeakMin);
    parsePeak("+0.0", true, Mixxx::ReplayGain::kPeakMin);
    parsePeak("-0.0", true, Mixxx::ReplayGain::kPeakMin);
    parsePeak("1", true, Mixxx::ReplayGain::kPeakClip);
    parsePeak("+1", true, Mixxx::ReplayGain::kPeakClip);
    parsePeak("1.0", true, Mixxx::ReplayGain::kPeakClip);
    parsePeak("+1.0", true, Mixxx::ReplayGain::kPeakClip);
    parsePeak("  0.12345  ", true, 0.12345);
    parsePeak("  1.2345", true, 1.2345);
}

TEST_F(ReplayGainTest, ParsePeakInvalid) {
    parsePeak("", false, Mixxx::ReplayGain::kPeakUndefined);
    parsePeak("-1", false, Mixxx::ReplayGain::kPeakUndefined);
    parsePeak("-0.12345", false, Mixxx::ReplayGain::kPeakUndefined);
    parsePeak("--1.0", false, Mixxx::ReplayGain::kPeakUndefined);
    parsePeak("+-1.0", false, Mixxx::ReplayGain::kPeakUndefined);
    parsePeak("-+1.0", false, Mixxx::ReplayGain::kPeakUndefined);
    parsePeak("++1.0", false, Mixxx::ReplayGain::kPeakUndefined);
    parsePeak("+abcde", false, Mixxx::ReplayGain::kPeakUndefined);
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
