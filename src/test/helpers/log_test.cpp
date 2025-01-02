#include "test/helpers/log_test.h"

QList<std::tuple<QtMsgType, QRegularExpression>> logMessagesExpected;

void logCapture(QtMsgType msgType, const QMessageLogContext&, const QString& msg) {
    for (int i = 0; i < logMessagesExpected.size(); i++) {
        if (std::get<QtMsgType>(logMessagesExpected[i]) == msgType &&
                std::get<QRegularExpression>(logMessagesExpected[i])
                        .match(msg)
                        .hasMatch()) {
            logMessagesExpected.removeAt(i);
            return;
        }
    }

    // Unexpected info or debug message aren't considered as a failure
    QString msgTypeStr;
    switch (msgType) {
    case QtDebugMsg:
    case QtInfoMsg:
        return;
    case QtWarningMsg:
        msgTypeStr = QStringLiteral("Warning: ") + msg;
        break;
    case QtCriticalMsg:
        msgTypeStr = QStringLiteral("Critical:") + msg;
        break;
    case QtFatalMsg:
        msgTypeStr = QStringLiteral("Fatal:") + msg;
        break;
    }
    QString errMsg("Got an unexpected log message: \n\t");
    QDebug strm(&errMsg);
    strm << msgTypeStr;
    FAIL() << errMsg.toStdString();
}
