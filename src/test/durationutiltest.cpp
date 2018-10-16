#include <gtest/gtest.h>

#include "util/duration.h"

#include <QtDebug>

namespace {

class DurationUtilTest : public testing::Test {
  protected:
    static QString adjustPrecision(
        QString withMilliseconds,
        mixxx::Duration::Precision precision) {
        switch (precision) {
        case mixxx::Duration::Precision::SECONDS:
        {
            return withMilliseconds.left(withMilliseconds.length() - 4);
        }
        case mixxx::Duration::Precision::CENTISECONDS:
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
            mixxx::Duration::formatSeconds(dSeconds, mixxx::Duration::Precision::SECONDS);
        const QString expectedSeconds =
                adjustPrecision(expectedMilliseconds, mixxx::Duration::Precision::SECONDS);
        EXPECT_EQ(expectedSeconds, actualSeconds);
        const QString expectedCentiseconds =
                adjustPrecision(expectedMilliseconds, mixxx::Duration::Precision::CENTISECONDS);
        const QString actualCentiseconds =
            mixxx::Duration::formatSeconds(dSeconds, mixxx::Duration::Precision::CENTISECONDS);
        EXPECT_EQ(expectedCentiseconds, actualCentiseconds);
        const QString actualMilliseconds =
            mixxx::Duration::formatSeconds(dSeconds, mixxx::Duration::Precision::MILLISECONDS);
        EXPECT_EQ(actualMilliseconds, actualMilliseconds);
    }
};

TEST_F(DurationUtilTest, FormatSecondsNegative) {
    EXPECT_EQ("?", mixxx::Duration::formatSeconds(-1, mixxx::Duration::Precision::SECONDS));
    EXPECT_EQ("?", mixxx::Duration::formatSeconds(-1, mixxx::Duration::Precision::CENTISECONDS));
    EXPECT_EQ("?", mixxx::Duration::formatSeconds(-1, mixxx::Duration::Precision::MILLISECONDS));
}

TEST_F(DurationUtilTest, FormatSeconds) {
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
