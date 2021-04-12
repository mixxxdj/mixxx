#include "util/logging.h"

#include <signal.h>
#include <stdio.h>

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QLoggingCategory>
#include <QMutex>
#include <QMutexLocker>
#include <QString>
#include <QThread>
#include <QtDebug>
#include <QtGlobal>
#include <cstring>

#include "controllers/controllerdebug.h"
#include "util/assert.h"

namespace mixxx {

// Initialize the log level with the default value
LogLevel g_logLevel = kLogLevelDefault;
LogLevel g_logFlushLevel = kLogFlushLevelDefault;

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
        fwrite(message.constData(), sizeof(char), message.size(), stderr);
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

/// Rotate existing logfiles and get the file path of the log file to write to.
/// May return an invalid/empty QString if the log directory does not exist.
inline QString rotateLogFilesAndGetFilePath(const QString& logDirPath) {
    if (logDirPath.isEmpty()) {
        fprintf(stderr, "No log directory specified!\n");
        return QString();
    }

    QDir logDir(logDirPath);
    if (!logDir.exists()) {
        fprintf(stderr,
                "Log directory %s does not exist!\n",
                logDir.absolutePath().toLocal8Bit().constData());
        return QString();
    }

    QString logFilePath;
    // Rotate old logfiles.
    for (int i = 9; i >= 0; --i) {
        const QString logFileName = (i == 0) ? QString("mixxx.log")
                                             : QString("mixxx.log.%1").arg(i);
        logFilePath = logDir.absoluteFilePath(logFileName);
        if (QFileInfo::exists(logFilePath)) {
            QString olderLogFilePath =
                    logDir.absoluteFilePath(QString("mixxx.log.%1").arg(i + 1));
            // This should only happen with number 10
            if (QFileInfo::exists(olderLogFilePath)) {
                QFile::remove(olderLogFilePath);
            }
            if (!QFile::rename(logFilePath, olderLogFilePath)) {
                fprintf(stderr,
                        "Error rolling over logfile %s\n",
                        logFilePath.toLocal8Bit().constData());
            }
        }
    }
    return logFilePath;
}

// Debug message handler which outputs to stderr and a logfile, prepending the
// thread name and log level.
void MessageHandler(QtMsgType type,
                    const QMessageLogContext&, const QString& input) {
    // For "]: " and '\n'.
    std::size_t baSize = 4;
    const char* tag = nullptr;
    bool shouldPrint = true;
    bool shouldFlush = false;
    bool isDebugAssert = false;
    bool isControllerDebug = false;
    switch (type) {
        case QtDebugMsg:
            tag = "Debug [";
            baSize += std::strlen(tag);
            isControllerDebug = input.startsWith(QLatin1String(
                ControllerDebug::kLogMessagePrefix));
            shouldPrint = Logging::enabled(LogLevel::Debug) ||
                    isControllerDebug;
            shouldFlush = Logging::flushing(LogLevel::Debug);
            break;
        case QtInfoMsg:
            tag = "Info [";
            baSize += std::strlen(tag);
            shouldPrint = Logging::enabled(LogLevel::Info);
            shouldFlush = Logging::flushing(LogLevel::Info);
            break;
        case QtWarningMsg:
            tag = "Warning [";
            baSize += std::strlen(tag);
            shouldPrint = Logging::enabled(LogLevel::Warning);
            shouldFlush = Logging::flushing(LogLevel::Warning);
            break;
        case QtCriticalMsg:
            tag = "Critical [";
            baSize += std::strlen(tag);
            shouldFlush = true;
            isDebugAssert = input.startsWith(QLatin1String(kDebugAssertPrefix));
            break;
        case QtFatalMsg:
            tag = "Fatal [";
            baSize += std::strlen(tag);
            shouldFlush = true;
            break;
        default:
            tag = "Unknown [";
            baSize += std::strlen(tag);
    }

    // qthread.cpp contains a Q_ASSERT that currentThread does not return
    // nullptr.
    QByteArray threadName = QThread::currentThread()
            ->objectName().toLocal8Bit();
    baSize += threadName.length();

    QByteArray input8Bit;
    if (isControllerDebug) {
        input8Bit = input.mid(ControllerDebug::kLogMessagePrefixLength + 1).toLocal8Bit();
    } else {
        input8Bit = input.toLocal8Bit();
    }
    baSize += input8Bit.size();

    QByteArray ba;
    ba.reserve(static_cast<int>(baSize));

    ba += tag;
    ba += threadName;
    ba += "]: ";
    ba += input8Bit;
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
        // The "%s" is intentional. See -Werror=format-security.
        qFatal("%s", input8Bit.constData());
        return;
#endif // MIXXX_DEBUG_ASSERTIONS_FATAL
    }

    writeToLog(ba, shouldPrint, shouldFlush);
}

}  // namespace

// static
void Logging::initialize(
        const QString& logDirPath,
        LogLevel logLevel,
        LogLevel logFlushLevel,
        LogFlags flags) {
    VERIFY_OR_DEBUG_ASSERT(!g_logfile.isOpen()) {
        // Somebody already called Logging::initialize.
        return;
    }

    setLogLevel(logLevel);

    QString logFilePath;
    if (flags.testFlag(LogFlag::LogToFile)) {
        logFilePath = rotateLogFilesAndGetFilePath(logDirPath);
    }

    if (logFilePath.isEmpty()) {
        // No need to flush anything
        g_logFlushLevel = LogLevel::Critical;
    } else {
        // Since the message handler is not installed yet, we can touch s_logfile
        // without the lock.
        g_logfile.setFileName(logFilePath);
        g_logfile.open(QIODevice::WriteOnly | QIODevice::Text);
        g_logFlushLevel = logFlushLevel;
    }

    g_debugAssertBreak = flags.testFlag(LogFlag::DebugAssertBreak);

    // Install the Qt message handler.
    qInstallMessageHandler(MessageHandler);

    // Ugly hack around distributions disabling debugging in Qt applications.
    // This restores the default Qt behavior. It is required for getting useful
    // logs from users and for developing controller mappings.
    // Fedora: https://bugzilla.redhat.com/show_bug.cgi?id=1227295
    // Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=886437
    // Ubuntu: https://bugs.launchpad.net/ubuntu/+source/qtbase-opensource-src/+bug/1731646
    // Somehow this causes a segfault on macOS though?? https://bugs.launchpad.net/mixxx/+bug/1871238
#ifdef __LINUX__
    QLoggingCategory::setFilterRules("*.debug=true\n"
                                     "qt.*.debug=false");
#endif
}

// static
void Logging::setLogLevel(LogLevel logLevel) {
    g_logLevel = logLevel;
}

// static
void Logging::shutdown() {
    // Reset the Qt message handler to default.
    qInstallMessageHandler(nullptr);

    // Even though we uninstalled the message handler, other threads may have
    // already entered it.
    QMutexLocker locker(&g_mutexLogfile);
    if (g_logfile.isOpen()) {
        g_logfile.close();
    }
}

// static
void Logging::flushLogFile() {
    QMutexLocker locker(&g_mutexLogfile);
    if (g_logfile.isOpen()) {
        g_logfile.flush();
    }
}

}  // namespace mixxx
