#pragma once

#include <gtest/gtest.h>

#include <QDebug>
#include <QList>
#include <QRegularExpression>
#include <tuple>

#define SETUP_LOG_CAPTURE() \
    qInstallMessageHandler(logCapture)

#define ASSERT_ALL_EXPECTED_MSG()                                                                  \
    if (!logMessagesExpected.isEmpty()) {                                                          \
        QString errMsg;                                                                            \
        QDebug strm(&errMsg);                                                                      \
        strm << logMessagesExpected.size() << "expected log messages didn't occur: \n";            \
        for (const auto& msg : std::as_const(logMessagesExpected))                                 \
            strm << "\t" << std::get<QtMsgType>(msg) << std::get<QRegularExpression>(msg) << "\n"; \
        FAIL() << errMsg.toStdString();                                                            \
    } else                                                                                         \
        logMessagesExpected.clear();

#define EXPECT_LOG_MSG(type, exp) \
    logMessagesExpected.push_back(std::make_tuple(type, QRegularExpression(exp)))

extern QList<std::tuple<QtMsgType, QRegularExpression>> logMessagesExpected;
void logCapture(QtMsgType msgType, const QMessageLogContext&, const QString& msg);
