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
        EXPECT_EQ(actualMilliseconds, actualMilliseconds);
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
        EXPECT_EQ(actualMilliseconds, actualMilliseconds);
    }

    void formatHectoSeconds(QString expectedMilliseconds, double dSeconds) {
        ASSERT_LE(4, expectedMilliseconds.length()); // 3 digits + 1 decimal point
        const QString actualSeconds =
            mixxx::Duration::formatHectoSeconds(dSeconds, mixxx::Duration::Precision::SECONDS);
        const QString expectedSeconds =
                adjustPrecision(expectedMilliseconds, mixxx::Duration::Precision::SECONDS);
        EXPECT_EQ(expectedSeconds, actualSeconds);
        const QString expectedCentiseconds =
                adjustPrecision(expectedMilliseconds, mixxx::Duration::Precision::CENTISECONDS);
        const QString actualCentiseconds =
            mixxx::Duration::formatHectoSeconds(dSeconds, mixxx::Duration::Precision::CENTISECONDS);
        EXPECT_EQ(expectedCentiseconds, actualCentiseconds);
        const QString actualMilliseconds =
            mixxx::Duration::formatHectoSeconds(dSeconds, mixxx::Duration::Precision::MILLISECONDS);
        EXPECT_EQ(actualMilliseconds, actualMilliseconds);
    }
};

TEST_F(DurationUtilTest, FormatSecondsNegative) {
    EXPECT_EQ("?", mixxx::Duration::formatTime(-1, mixxx::Duration::Precision::SECONDS));
    EXPECT_EQ("?", mixxx::Duration::formatTime(-1, mixxx::Duration::Precision::CENTISECONDS));
    EXPECT_EQ("?", mixxx::Duration::formatTime(-1, mixxx::Duration::Precision::MILLISECONDS));
}

TEST_F(DurationUtilTest, formatTime) {
    formatTime("00:00.000", 0);
    formatTime("00:01.000", 1);
    formatTime("00:59.000", 59);
    formatTime("01:00.000", 60);
    formatTime("01:01.123", 61.1234);
    formatTime("59:59.999", 3599.999);
    formatTime("01:00:00.000", 3600);
    formatTime("23:59:59.000", 24 * 3600 - 1);
    formatTime("24:00:00.000", 24 * 3600);
    formatTime("24:00:01.000", 24 * 3600 + 1);
    formatTime("25:00:01.000", 25 * 3600 + 1);
}

TEST_F(DurationUtilTest, formatSecond) {
    formatSeconds("0.000", 0);
    formatSeconds("1.000", 1);
    formatSeconds("59.000", 59);
    formatSeconds("60.000", 60);
    formatSeconds("321.123", 321.1234);
}


TEST_F(DurationUtilTest, FormatKiloSeconds) {
    formatKiloSeconds(QString::fromUtf8("0\u2009000\u002E000"), 0);
    formatKiloSeconds(QString::fromUtf8("0\u2009001\u002E000"), 1);
    formatKiloSeconds(QString::fromUtf8("0\u2009001\u002E500"), 1.5);
    formatKiloSeconds(QString::fromUtf8("0\u2009001\u002E510"), 1.51);
    formatKiloSeconds(QString::fromUtf8("0\u2009001\u002E490"), 1.49);
    formatKiloSeconds(QString::fromUtf8("0\u2009059\u002E000"), 59);
    formatKiloSeconds(QString::fromUtf8("0\u2009060\u002E000"), 60);
    formatKiloSeconds(QString::fromUtf8("0\u2009061\u002E123"), 61.1234);
    formatKiloSeconds(QString::fromUtf8("0\u2009999\u002E990"), 999.99);
    formatKiloSeconds(QString::fromUtf8("1\u2009000\u002E000"), 1000.00);
    formatKiloSeconds(QString::fromUtf8("86\u2009400\u002E000"), 24 * 3600);
}

TEST_F(DurationUtilTest, FormatHectoSeconds) {
    formatHectoSeconds(QString::fromUtf8("0\u231E00\u002E000"), 0);
    formatHectoSeconds(QString::fromUtf8("0\u231E01\u002E000"), 1);
    formatHectoSeconds(QString::fromUtf8("0\u231E01\u002E500"), 1.5);
    formatHectoSeconds(QString::fromUtf8("0\u231E01\u002E510"), 1.51);
    formatHectoSeconds(QString::fromUtf8("0\u231E01\u002E490"), 1.49);
    formatHectoSeconds(QString::fromUtf8("0\u231E59\u002E000"), 59);
    formatHectoSeconds(QString::fromUtf8("0\u231E60\u002E000"), 60);
    formatHectoSeconds(QString::fromUtf8("0\u231E61\u002E123"), 61.1234);
    formatHectoSeconds(QString::fromUtf8("9\u231E99\u002E990"), 999.99);
    formatHectoSeconds(QString::fromUtf8("10\u231E00\u002E000"), 1000.00);
    formatHectoSeconds(QString::fromUtf8("864\u231E00\u002E000"), 24 * 3600);
}


} // anonymous namespace
