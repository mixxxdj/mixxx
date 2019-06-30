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

    void formatTime(QString expectedMilliseconds, double dSeconds) {
        ASSERT_LE(4, expectedMilliseconds.length()); // 3 digits + 1 decimal point
        const QString actualSeconds =
            mixxx::Duration::formatTime(dSeconds, mixxx::Duration::Precision::SECONDS);
        const QString expectedSeconds =
                adjustPrecision(expectedMilliseconds, mixxx::Duration::Precision::SECONDS);
        EXPECT_EQ(expectedSeconds, actualSeconds);
        const QString expectedCentiseconds =
                adjustPrecision(expectedMilliseconds, mixxx::Duration::Precision::CENTISECONDS);
        const QString actualCentiseconds =
            mixxx::Duration::formatTime(dSeconds, mixxx::Duration::Precision::CENTISECONDS);
        EXPECT_EQ(expectedCentiseconds, actualCentiseconds);
        const QString actualMilliseconds =
            mixxx::Duration::formatTime(dSeconds, mixxx::Duration::Precision::MILLISECONDS);
        EXPECT_EQ(expectedMilliseconds, actualMilliseconds);
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
        EXPECT_EQ(expectedMilliseconds, actualMilliseconds);
    }

    void formatSecondsLong(QString expectedMilliseconds, double dSeconds) {
        ASSERT_LE(4, expectedMilliseconds.length()); // 3 digits + 1 decimal point
        const QString actualSeconds =
            mixxx::Duration::formatSecondsLong(dSeconds, mixxx::Duration::Precision::SECONDS);
        const QString expectedSeconds =
                adjustPrecision(expectedMilliseconds, mixxx::Duration::Precision::SECONDS);
        EXPECT_EQ(expectedSeconds, actualSeconds);
        const QString expectedCentiseconds =
                adjustPrecision(expectedMilliseconds, mixxx::Duration::Precision::CENTISECONDS);
        const QString actualCentiseconds =
            mixxx::Duration::formatSecondsLong(dSeconds, mixxx::Duration::Precision::CENTISECONDS);
        EXPECT_EQ(expectedCentiseconds, actualCentiseconds);
        const QString actualMilliseconds =
            mixxx::Duration::formatSecondsLong(dSeconds, mixxx::Duration::Precision::MILLISECONDS);
        EXPECT_EQ(expectedMilliseconds, actualMilliseconds);
    }

    void formatKiloSeconds(QString expectedMilliseconds, double dSeconds) {
        ASSERT_LE(4, expectedMilliseconds.length()); // 3 digits + 1 decimal point
        const QString actualSeconds =
            mixxx::Duration::formatKiloSeconds(dSeconds, mixxx::Duration::Precision::SECONDS);
        const QString expectedSeconds =
                adjustPrecision(expectedMilliseconds, mixxx::Duration::Precision::SECONDS);
        EXPECT_EQ(expectedSeconds, actualSeconds);
        const QString expectedCentiseconds =
                adjustPrecision(expectedMilliseconds, mixxx::Duration::Precision::CENTISECONDS);
        const QString actualCentiseconds =
            mixxx::Duration::formatKiloSeconds(dSeconds, mixxx::Duration::Precision::CENTISECONDS);
        EXPECT_EQ(expectedCentiseconds, actualCentiseconds);
        const QString actualMilliseconds =
            mixxx::Duration::formatKiloSeconds(dSeconds, mixxx::Duration::Precision::MILLISECONDS);
        EXPECT_EQ(expectedMilliseconds, actualMilliseconds);
    }
};

TEST_F(DurationUtilTest, FormatSecondsNegative) {
    EXPECT_EQ("?", mixxx::Duration::formatTime(-1, mixxx::Duration::Precision::SECONDS));
    EXPECT_EQ("?", mixxx::Duration::formatTime(-1, mixxx::Duration::Precision::CENTISECONDS));
    EXPECT_EQ("?", mixxx::Duration::formatTime(-1, mixxx::Duration::Precision::MILLISECONDS));
}

TEST_F(DurationUtilTest, formatTime) {
    formatTime("0:00.000", 0);
    formatTime("0:01.000", 1);
    formatTime("0:59.000", 59);
    formatTime("1:00.000", 60);
    formatTime("1:01.123", 61.1234);
    formatTime("59:59.999", 3599.999);
    formatTime("1:00:00.000", 3600);
    formatTime("23:59:59.000", 24 * 3600 - 1);
    formatTime("24:00:00.000", 24 * 3600);
    formatTime("24:00:01.000", 24 * 3600 + 1);
    formatTime("25:00:01.000", 25 * 3600 + 1);
}

TEST_F(DurationUtilTest, formatSeconds) {
    formatSeconds("0.000", 0);
    formatSeconds("1.000", 1);
    formatSeconds("59.000", 59);
    formatSeconds("60.000", 60);
    formatSeconds("321.123", 321.1234);
}

TEST_F(DurationUtilTest, formatSecondsLong) {
    formatSecondsLong("000.000", 0);
    formatSecondsLong("001.000", 1);
    formatSecondsLong("059.000", 59);
    // TODO() rounding is done with single precision which fails using
    // these high values. Find out why. Tested on Ubuntu Trusty.
    formatSecondsLong("321.123", 321.1234);
    formatSecondsLong("321.124", 321.1236);
    formatSecondsLong("4321.123", 4321.1230);
}


TEST_F(DurationUtilTest, FormatKiloSeconds) {
    formatKiloSeconds(QStringLiteral(u"0.000\u2009000"), 0);
    formatKiloSeconds(QStringLiteral(u"0.001\u2009000"), 1);
    formatKiloSeconds(QStringLiteral(u"0.001\u2009490"), 1.49);
    formatKiloSeconds(QStringLiteral(u"0.059\u2009000"), 59);
    formatKiloSeconds(QStringLiteral(u"0.061\u2009123"), 61.1234);
    formatKiloSeconds(QStringLiteral(u"0.999\u2009990"), 999.99);
    formatKiloSeconds(QStringLiteral(u"1.000\u2009000"), 1000.00);
    formatKiloSeconds(QStringLiteral(u"86.400\u2009000"), 24 * 3600);
}


} // anonymous namespace
