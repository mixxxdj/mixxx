#pragma once

#include <gtest/gtest.h>

#include <QDebug>
#include <QList>
#include <QRegularExpression>
#include <tuple>

#define ASSERT_ALL_EXPECTED_MSG()                                \
    {                                                            \
        QString errMsg = LogCaptureGuard::clearExpectedGetMsg(); \
        if (!errMsg.isEmpty()) {                                 \
            FAIL() << errMsg.toStdString();                      \
        }                                                        \
    }

#define EXPECT_LOG_MSG(type, exp) LogCaptureGuard::expect(type, exp)

class LogCaptureGuard {
  public:
    LogCaptureGuard();
    ~LogCaptureGuard();

    static void expect(QtMsgType type, const QString& exp);
    static QString clearExpectedGetMsg();
};
