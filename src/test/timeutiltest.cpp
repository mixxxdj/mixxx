#include <gtest/gtest.h>

#include "util/time.h"

#include <QtDebug>

namespace {

class TimeUtilTest : public testing::Test {
  protected:

    TimeUtilTest() {
    }

    virtual void SetUp() {
    }

    virtual void TearDown() {
    }
    
    static QString adjustPrecision(
        QString withMilliseconds,
        Time::Precision precision) {
        switch (precision) {
        case Time::Precision::SECONDS:
        {
            return withMilliseconds.left(withMilliseconds.length() - 4);
        }
        case Time::Precision::CENTISECONDS:
        {
            return withMilliseconds.left(withMilliseconds.length() - 1);
        }
        default:
            return withMilliseconds;
        }
    }
    
    void formatSeconds(QString expectedMilliseconds, double dSeconds) {
        ASSERT_LE(4, expectedMilliseconds.length()); // 3 digits + 1 decimal point
        const QString actualSeconds =
            Time::formatSeconds(dSeconds, Time::Precision::SECONDS);
        const QString expectedSeconds =
                adjustPrecision(expectedMilliseconds, Time::Precision::SECONDS);
        EXPECT_EQ(expectedSeconds, actualSeconds);
        const QString expectedCentiseconds =
                adjustPrecision(expectedMilliseconds, Time::Precision::CENTISECONDS);
        const QString actualCentiseconds =
            Time::formatSeconds(dSeconds, Time::Precision::CENTISECONDS);
        EXPECT_EQ(expectedCentiseconds, actualCentiseconds);
        const QString actualMilliseconds =
            Time::formatSeconds(dSeconds, Time::Precision::MILLISECONDS);
        EXPECT_EQ(actualMilliseconds, actualMilliseconds);
    }
};

TEST_F(TimeUtilTest, FormatSecondsNegative) {
    EXPECT_EQ("?", Time::formatSeconds(-1, Time::Precision::SECONDS));
    EXPECT_EQ("?", Time::formatSeconds(-1, Time::Precision::CENTISECONDS));
    EXPECT_EQ("?", Time::formatSeconds(-1, Time::Precision::MILLISECONDS));
}

TEST_F(TimeUtilTest, FormatSeconds) {
    formatSeconds("00:00.000", 0);
    formatSeconds("00:01.000", 1);
    formatSeconds("00:59.000", 59);
    formatSeconds("01:00.000", 60);
    formatSeconds("01:01.123", 61.1234);
    formatSeconds("59:59.999", 3599.999);
    formatSeconds("01:00:00.000", 3600);
    formatSeconds("23:59:59.000", 24 * 3600 - 1);
    formatSeconds("1d, 00:00:00.000", 24 * 3600);
}

} // anonymous namespace
