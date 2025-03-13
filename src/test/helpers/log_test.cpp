#include "test/helpers/log_test.h"

namespace {

QList<std::tuple<QtMsgType, QRegularExpression>> s_logMessagesExpected;
QtMessageHandler s_oldHandler;

void callOldHandler(QtMsgType msgType, const QMessageLogContext& context, const QString& msg) {
    // QtMessageHandler is a typedef for a pointer to a function
    s_oldHandler(msgType, context, msg);
}

void logCapture(QtMsgType msgType, const QMessageLogContext& context, const QString& msg) {
    // First log msg as usual
    callOldHandler(msgType, context, msg);

    // Return without failing if the msg has been expected
    for (int i = 0; i < s_logMessagesExpected.size(); i++) {
        if (std::get<QtMsgType>(s_logMessagesExpected[i]) == msgType &&
                std::get<QRegularExpression>(s_logMessagesExpected[i])
                        .match(msg)
                        .hasMatch()) {
            s_logMessagesExpected.removeAt(i);
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
        msgTypeStr = QStringLiteral("Critical: ") + msg;
        break;
    case QtFatalMsg:
        msgTypeStr = QStringLiteral("Fatal: ") + msg;
        break;
    }
    QString errMsg("Got an unexpected log message: \n\t");
    QDebug strm(&errMsg);
    strm << msgTypeStr;
    FAIL() << errMsg.toStdString();
}

} // namespace

LogCaptureGuard::LogCaptureGuard() {
    s_oldHandler = qInstallMessageHandler(logCapture);
}

LogCaptureGuard::~LogCaptureGuard() {
    s_logMessagesExpected.clear();
    qInstallMessageHandler(s_oldHandler);
}

// static
void LogCaptureGuard::expect(QtMsgType type, const QString& exp) {
    s_logMessagesExpected.push_back(std::make_tuple(type, QRegularExpression(exp)));
}

// static
QString LogCaptureGuard::clearExpectedGetMsg() {
    QString errMsg;
    if (!s_logMessagesExpected.isEmpty()) {
        QDebug strm(&errMsg);
        strm << s_logMessagesExpected.size() << "expected log messages didn't occur: \n";
        for (const auto& [type, regex] : std::as_const(s_logMessagesExpected)) {
            QString msgTypeStr;
            switch (type) {
            case QtDebugMsg:
                msgTypeStr = QStringLiteral("Warning: ") + regex.pattern();
                break;
            case QtInfoMsg:
                msgTypeStr = QStringLiteral("Warning: ") + regex.pattern();
                break;
            case QtWarningMsg:
                msgTypeStr = QStringLiteral("Warning: ") + regex.pattern();
                break;
            case QtCriticalMsg:
                msgTypeStr = QStringLiteral("Critical: ") + regex.pattern();
                break;
            case QtFatalMsg:
                msgTypeStr = QStringLiteral("Fatal: ") + regex.pattern();
                break;
            }
            strm << "\t" << msgTypeStr << "\n";
        }
        s_logMessagesExpected.clear();
    }
    return errMsg;
}
