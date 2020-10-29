#include "util/logging.h"

#include <signal.h>
#include <stdio.h>

#include <QByteArray>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QLoggingCategory>
#include <QMutex>
#include <QMutexLocker>
#include <QString>
#include <QTextStream>
#include <QThread>

#include "controllers/controllerdebug.h"
#include "util/assert.h"

namespace {

/// Mutex guarding s_logfile.
QMutex s_mutexLogfile;

/// Mutex guarding stderr.
QMutex s_mutexStdErr;

// The file handle for Mixxx's log file.
QFile s_logfile;

// The log level.
// Whether to break on debug assertions.
bool s_debugAssertBreak = false;

const QString kThreadNamePattern = QStringLiteral("{{threadname}}");
const QString kDefaultMessagePattern = QStringLiteral("%{type} [") +
        kThreadNamePattern + QStringLiteral("] %{message}");

const QLoggingCategory kDefaultLoggingCategory = QLoggingCategory(nullptr);

enum class WriteFlag {
    None = 0,
    StdErr = 1 << 0,
    File = 1 << 1,
    Flush = 1 << 2,
    All = StdErr | File | Flush,
};

Q_DECLARE_FLAGS(WriteFlags, WriteFlag)

// Clang 10 complains about an unused function introduced by
// Q_DECLARE_OPERATORS_FOR_FLAGS
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#endif
Q_DECLARE_OPERATORS_FOR_FLAGS(WriteFlags)
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

/// Format message for writing into log file (ignores QT_MESSAGE_PATTERN,
/// because logfiles should have a fixed format).
inline QString formatLogFileMessage(
        QtMsgType type,
        const QString& message,
        const QString& threadName) {
    QString levelName;
    switch (type) {
    case QtDebugMsg:
        levelName = QStringLiteral("Debug");
        break;
    case QtInfoMsg:
        levelName = QStringLiteral("Info");
        break;
    case QtWarningMsg:
        levelName = QStringLiteral("Warning");
        break;
    case QtCriticalMsg:
        levelName = QStringLiteral("Critical");
        break;
    case QtFatalMsg:
        levelName = QStringLiteral("Fatal");
        break;
    }

    return levelName + QStringLiteral(" [") + threadName + QStringLiteral("] ") + message;
}

/// Actually write a log message to a file.
inline void writeToFile(
        QtMsgType type,
        const QString& message,
        const QString& threadName,
        bool flush) {
    QString formattedMessageStr =
            formatLogFileMessage(type, message, threadName) +
            QChar('\n');
    QByteArray formattedMessage = formattedMessageStr.toLocal8Bit();

    QMutexLocker locked(&s_mutexLogfile);
    // Writing to a closed QFile could cause an infinite recursive loop
    // by logging to qWarning!
    if (s_logfile.isOpen()) {
        const int written = s_logfile.write(formattedMessage);
        Q_UNUSED(written);
        DEBUG_ASSERT(written == formattedMessage.size());
        if (flush) {
            const bool flushed = s_logfile.flush();
            Q_UNUSED(flushed);
            DEBUG_ASSERT(flushed);
        }
    }
}

/// Actually write a log message to stderr.
inline void writeToStdErr(
        QtMsgType type,
        const QMessageLogContext& context,
        const QString& message,
        const QString& threadName,
        bool flush) {
    QString formattedMessageStr = qFormatLogMessage(type, context, message) + QChar('\n');
    const QByteArray formattedMessage =
            formattedMessageStr.replace(kThreadNamePattern, threadName)
                    .toLocal8Bit();

    QMutexLocker locked(&s_mutexStdErr);
    const size_t written = fwrite(
            formattedMessage.constData(), sizeof(char), formattedMessage.size(), stderr);
    Q_UNUSED(written);
    DEBUG_ASSERT(written == static_cast<size_t>(formattedMessage.size()));
    if (flush) {
        // Flushing stderr might not be necessary, because message
        // should end with a newline character. Flushing occurs
        // only infrequently (log level >= Critical), so better safe
        // than sorry.
        const int ret = fflush(stderr);
        Q_UNUSED(ret);
        DEBUG_ASSERT(ret == 0);
    }
}

/// Handles writing to stderr and the log file.
inline void writeToLog(
        QtMsgType type,
        const QMessageLogContext& context,
        const QString& message,
        WriteFlags flags) {
    DEBUG_ASSERT(!message.isEmpty());
    DEBUG_ASSERT(flags & (WriteFlag::StdErr | WriteFlag::File));

    QString threadName = QThread::currentThread()->objectName();
    if (threadName.isEmpty()) {
        QTextStream textStream(&threadName);
        textStream << QThread::currentThread();
    }

    const bool flush = flags & WriteFlag::Flush;
    if (flags & WriteFlag::StdErr) {
        writeToStdErr(type, context, message, threadName, flush);
    }
    if (flags & WriteFlag::File) {
        writeToFile(type, message, threadName, flush);
    }
}

} // anonymous namespace

namespace mixxx {

namespace {

// Debug message handler which outputs to stderr and a logfile,
// prepending the thread name, log category, and log level.
void handleMessage(
        QtMsgType type,
        const QMessageLogContext& context,
        const QString& input) {
    const char* levelName = nullptr;
    WriteFlags writeFlags = WriteFlag::None;
    bool isDebugAssert = false;
    bool isControllerDebug = false;
    switch (type) {
    case QtDebugMsg:
        levelName = "Debug";
        isControllerDebug =
                input.startsWith(QLatin1String(
                        ControllerDebug::kLogMessagePrefix));
        if (isControllerDebug ||
                Logging::enabled(LogLevel::Debug)) {
            writeFlags |= WriteFlag::StdErr;
            writeFlags |= WriteFlag::File;
        }
        if (Logging::shouldFlush(LogLevel::Debug)) {
            writeFlags |= WriteFlag::Flush;
        }
        // TODO: Remove the following line.
        // Do not write debug log messages into log file if log level
        // Debug is not enabled starting with release 2.4.0! Until then
        // write debug messages unconditionally into the log file
        writeFlags |= WriteFlag::File;
        break;
    case QtInfoMsg:
        levelName = "Info";
        if (Logging::enabled(LogLevel::Info)) {
            writeFlags |= WriteFlag::StdErr;
        }
        if (Logging::shouldFlush(LogLevel::Info)) {
            writeFlags |= WriteFlag::Flush;
        }
        // Write unconditionally into log file
        writeFlags |= WriteFlag::File;
        break;
    case QtWarningMsg:
        levelName = "Warning";
        if (Logging::enabled(LogLevel::Warning)) {
            writeFlags |= WriteFlag::StdErr;
        }
        if (Logging::shouldFlush(LogLevel::Warning)) {
            writeFlags |= WriteFlag::Flush;
        }
        // Write unconditionally into log file
        writeFlags |= WriteFlag::File;
        break;
    case QtCriticalMsg:
        levelName = "Critical";
        writeFlags = WriteFlag::All;
        isDebugAssert = input.startsWith(QLatin1String(kDebugAssertPrefix));
        break;
    case QtFatalMsg:
        levelName = "Fatal";
        writeFlags = WriteFlag::All;
        break;
    }
    if (!writeFlags) {
        // Ignore message for disabled log level
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(levelName) {
        return;
    }

    if (isDebugAssert) {
        if (s_debugAssertBreak) {
            writeToLog(type, context, input, WriteFlag::All);
            raise(SIGINT);
            // When the debugger returns, continue normally.
            return;
        }
        // If debug assertions are non-fatal, we will fall through to the normal
        // writeToLog case below.
#ifdef MIXXX_DEBUG_ASSERTIONS_FATAL
        // re-send as fatal.
        // The "%s" is intentional. See -Werror=format-security.
        qFatal("%s", input.toLocal8Bit().constData());
        return;
#endif // MIXXX_DEBUG_ASSERTIONS_FATAL
    }

    writeToLog(type, context, input, writeFlags);
}

} // anonymous namespace

// Initialize the log level with the default value
LogLevel Logging::s_logLevel = kLogLevelDefault;
LogLevel Logging::s_logFlushLevel = kLogFlushLevelDefault;

// static
void Logging::initialize(
        const QDir& logDir,
        LogLevel logLevel,
        LogLevel logFlushLevel,
        bool debugAssertBreak) {
    VERIFY_OR_DEBUG_ASSERT(!s_logfile.isOpen()) {
        // Somebody already called Logging::initialize.
        return;
    }

    setLogLevel(logLevel);

    if (logDir.exists()) {
        QString logFileName;

        // Rotate old logfiles.
        for (int i = 9; i >= 0; --i) {
            if (i == 0) {
                logFileName = logDir.filePath("mixxx.log");
            } else {
                logFileName = logDir.filePath(QString("mixxx.log.%1").arg(i));
            }
            QFileInfo logbackup(logFileName);
            if (logbackup.exists()) {
                QString olderlogname =
                        logDir.filePath(QString("mixxx.log.%1").arg(i + 1));
                // This should only happen with number 10
                if (QFileInfo::exists(olderlogname)) {
                    QFile::remove(olderlogname);
                }
                if (!QFile::rename(logFileName, olderlogname)) {
                    fprintf(stderr,
                            "Error rolling over logfile %s",
                            logFileName.toLocal8Bit().constData());
                }
            }
        }
        // Since the message handler is not installed yet, we can touch s_logfile
        // without the lock.
        s_logfile.setFileName(logFileName);
        s_logfile.open(QIODevice::WriteOnly | QIODevice::Text);
        s_logFlushLevel = logFlushLevel;
    } else {
        qInfo() << "No directory for writing a log file";
        // No need to flush anything
        s_logFlushLevel = LogLevel::Critical;
    }

    s_debugAssertBreak = debugAssertBreak;

    if (qgetenv("QT_MESSAGE_PATTERN").isEmpty()) {
        qSetMessagePattern(kDefaultMessagePattern);
    }

    // Install the Qt message handler.
    qInstallMessageHandler(handleMessage);

    // Ugly hack around distributions disabling debugging in Qt applications.
    // This restores the default Qt behavior. It is required for getting useful
    // logs from users and for developing controller mappings.
    // Fedora: https://bugzilla.redhat.com/show_bug.cgi?id=1227295
    // Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=886437
    // Ubuntu: https://bugs.launchpad.net/ubuntu/+source/qtbase-opensource-src/+bug/1731646
    // Somehow this causes a segfault on macOS though?? https://bugs.launchpad.net/mixxx/+bug/1871238
#ifdef __LINUX__
    QLoggingCategory::setFilterRules(
            "*.debug=true\n"
            "qt.*.debug=false");
#endif
}

// static
void Logging::shutdown() {
    // Reset the Qt message handler to default.
    qInstallMessageHandler(nullptr);

    // Even though we uninstalled the message handler, other threads may have
    // already entered it.
    QMutexLocker locker(&s_mutexLogfile);
    if (s_logfile.isOpen()) {
        s_logfile.close();
    }
}

// static
void Logging::flushLogFile() {
    QMutexLocker locker(&s_mutexLogfile);
    if (s_logfile.isOpen()) {
        s_logfile.flush();
    }
}

} // namespace mixxx
