#include "util/logging.h"

#include <stdio.h>
#include <signal.h>

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QMutex>
#include <QMutexLocker>
#include <QString>
#include <QThread>
#include <QtDebug>
#include <QtGlobal>

#include "controllers/controllerdebug.h"
#include "util/assert.h"

namespace mixxx {

// Initialize the log level with the default value
LogLevel Logging::s_logLevel = LogLevel::Default;

namespace {

// Mutex guarding g_logfile.
QMutex g_mutexLogfile;
// The file handle for Mixxx's log file.
QFile g_logfile;
// The log level.
// Whether to break on debug assertions.
bool g_debugAssertBreak = false;

// Handles actually writing to stderr and the log.
inline void writeToLog(const QByteArray& message, bool shouldPrint,
                       bool shouldFlush) {
    if (shouldPrint) {
        fwrite(message.constData(), sizeof(char), message.size() + 1, stderr);
    }

    QMutexLocker locker(&g_mutexLogfile);
    // Writing to a closed QFile can cause a recursive loop, since it prints an
    // error using qWarning.
    if (g_logfile.isOpen()) {
        g_logfile.write(message);
        if (shouldFlush) {
            g_logfile.flush();
        }
    }
}

// Debug message handler which outputs to stderr and a logfile, prepending the
// thread name and log level.
void MessageHandler(QtMsgType type,
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
                    const char* input) {
#else
                    const QMessageLogContext&, const QString& input) {
#endif
    // For "]: " and '\n'.
    size_t baSize = 4;
    const char* tag = nullptr;
    bool shouldPrint = true;
    bool shouldFlush = false;
    bool isDebugAssert = false;
    bool isControllerDebug = false;
    switch (type) {
        case QtDebugMsg:
            tag = "Debug [";
            baSize += strlen(tag);
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
            isControllerDebug = strncmp(input, ControllerDebug::kLogMessagePrefix,
                                        strlen(ControllerDebug::kLogMessagePrefix)) == 0;
#else
            isControllerDebug = input.startsWith(QLatin1String(
                ControllerDebug::kLogMessagePrefix));
#endif
            shouldPrint = Logging::enabled(LogLevel::Debug) ||
                    isControllerDebug;
            break;
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
        case QtInfoMsg:
            tag = "Info [";
            baSize += strlen(tag);
            shouldPrint = Logging::enabled(LogLevel::Info);
            break;
#endif
        case QtWarningMsg:
            tag = "Warning [";
            baSize += strlen(tag);
            shouldPrint = Logging::enabled(LogLevel::Warning);
            break;
        case QtCriticalMsg:
            tag = "Critical [";
            baSize += strlen(tag);
            shouldFlush = true;
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
            isDebugAssert = strncmp(input, kDebugAssertPrefix,
                                    strlen(kDebugAssertPrefix)) == 0;
#else
            isDebugAssert = input.startsWith(QLatin1String(kDebugAssertPrefix));
#endif
            break;
        case QtFatalMsg:
            tag = "Fatal [";
            baSize += strlen(tag);
            shouldFlush = true;
            break;
        default:
            tag = "Unknown [";
            baSize += strlen(tag);
    }

    // qthread.cpp contains a Q_ASSERT that currentThread does not return
    // nullptr.
    QByteArray threadName = QThread::currentThread()
            ->objectName().toLocal8Bit();
    baSize += threadName.length();

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    const char* inputOffset = input;
    if (isControllerDebug) {
        inputOffset += strlen(ControllerDebug::kLogMessagePrefix) + 1;
    }
    baSize += strlen(inputOffset);
#else
    QByteArray input8Bit;
    if (isControllerDebug) {
        input8Bit = input.mid(strlen(ControllerDebug::kLogMessagePrefix) + 1).toLocal8Bit();
    } else {
        input8Bit = input.toLocal8Bit();
    }
    baSize += input8Bit.size();
#endif

    QByteArray ba;
    ba.reserve(baSize);

    ba += tag;
    ba += threadName;
    ba += "]: ";
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    ba += inputOffset;
#else
    ba += input8Bit;
#endif
    ba += '\n';

    if (isDebugAssert) {
        if (g_debugAssertBreak) {
            writeToLog(ba, true, true);
            raise(SIGINT);
            // If the debugger returns, continue normally.
            return;
        }
        // If debug assertions are non-fatal, we will fall through to the normal
        // writeToLog case below.
#ifdef MIXXX_DEBUG_ASSERTIONS_FATAL
        // re-send as fatal.
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
        // The "%s" is intentional. See -Werror=format-security.
        qFatal("%s", input);
#else
        // The "%s" is intentional. See -Werror=format-security.
        qFatal("%s", input8Bit.constData());
#endif // QT_VERSION
        return;
#endif // MIXXX_DEBUG_ASSERTIONS_FATAL
    }

    writeToLog(ba, shouldPrint, shouldFlush);
}

}  // namespace

// static
void Logging::initialize(const QDir& settingsDir, LogLevel logLevel,
                         bool debugAssertBreak) {
    VERIFY_OR_DEBUG_ASSERT(!g_logfile.isOpen()) {
        // Somebody already called Logging::initialize.
        return;
    }

    s_logLevel = logLevel;

    QString logFileName;

    // Rotate old logfiles.
    for (int i = 9; i >= 0; --i) {
        if (i == 0) {
            logFileName = settingsDir.filePath("mixxx.log");
        } else {
            logFileName = settingsDir.filePath(QString("mixxx.log.%1").arg(i));
        }
        QFileInfo logbackup(logFileName);
        if (logbackup.exists()) {
            QString olderlogname =
                    settingsDir.filePath(QString("mixxx.log.%1").arg(i + 1));
            // This should only happen with number 10
            if (QFileInfo(olderlogname).exists()) {
                QFile::remove(olderlogname);
            }
            if (!QFile::rename(logFileName, olderlogname)) {
                fprintf(stderr, "Error rolling over logfile %s",
                        logFileName.toLocal8Bit().constData());
            }
        }
    }

    // Since the message handler is not installed yet, we can touch g_logfile
    // without the lock.
    g_logfile.setFileName(logFileName);
    g_logfile.open(QIODevice::WriteOnly | QIODevice::Text);
    g_debugAssertBreak = debugAssertBreak;

    // Install the Qt message handler.
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    qInstallMsgHandler(MessageHandler);
#else
    qInstallMessageHandler(MessageHandler);
#endif
}

// static
void Logging::shutdown() {
    // Reset the Qt message handler to default.
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    qInstallMsgHandler(nullptr);
#else
    qInstallMessageHandler(nullptr);
#endif

    // Even though we uninstalled the message handler, other threads may have
    // already entered it.
    QMutexLocker locker(&g_mutexLogfile);
    if (g_logfile.isOpen()) {
        g_logfile.close();
    }
}

}  // namespace mixxx
