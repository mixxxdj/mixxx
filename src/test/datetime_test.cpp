#include "util/datetime.h"

#include <gtest/gtest.h>

#include <QDate>
#include <QLocale>

namespace mixxx {

TEST(DateTimeTest, FormatDate) {
    QDate date(2023, 10, 25);

    // Test ISO
    EXPECT_EQ(formatDate(date, "yyyy-MM-dd"), "2023-10-25");

    // Test Custom
    EXPECT_EQ(formatDate(date, "dd.MM.yy"), "25.10.23");

    // Test Native (empty string)
    // Should fallback to QLocale::ShortFormat
    QString native = QLocale().toString(date, QLocale::ShortFormat);
    EXPECT_EQ(formatDate(date, ""), native);
}

TEST(DateTimeTest, FormatDateTime) {
    QDateTime dt(QDate(2023, 10, 25), QTime(14, 30, 0));

    // Test Custom
    EXPECT_EQ(formatDateTime(dt, "yyyy-MM-dd HH:mm"), "2023-10-25 14:30");

    // Test Native (empty string)
    QString native = QLocale().toString(dt, QLocale::ShortFormat);
    EXPECT_EQ(formatDateTime(dt, ""), native);
}

} // namespace mixxx
