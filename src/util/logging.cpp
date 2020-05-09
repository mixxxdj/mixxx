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

// Mutex guarding s_logfile.
QMutex s_mutexLogfile;

// The file handle for Mixxx's log file.
QFile s_logfile;

// The log level.
// Whether to break on debug assertions.
bool s_debugAssertBreak = false;

const QLoggingCategory kDefaultLoggingCategory = QLoggingCategory(nullptr);

enum class WriteFlag {
    None = 0,
    StdErr = 1 << 0,
    File = 1 << 1,
    Flush = 1 << 2,
    All = StdErr | File | Flush,
};

Q_DECLARE_FLAGS(WriteFlags, WriteFlag)

Q_DECLARE_OPERATORS_FOR_FLAGS(WriteFlags)

// Handles actually writing to stderr and the log file.
inline void writeToLog(
        const QByteArray& message,
        WriteFlags flags) {
    DEBUG_ASSERT(!message.isEmpty());
    DEBUG_ASSERT(flags & (WriteFlag::StdErr | WriteFlag::File));
    if (flags & WriteFlag::StdErr) {
        const int written = fwrite(
                message.constData(), sizeof(char), message.size(), stderr);
        Q_UNUSED(written);
        DEBUG_ASSERT(written == message.size());
        if (flags & WriteFlag::Flush) {
            // Flushing stderr might not be necessary, because message
            // should end with a newline character. Flushing occcurs
            // only infrequently (log level >= Critical), so better safe
            // than sorry.
            const int ret = fflush(stderr);
            Q_UNUSED(ret);
            DEBUG_ASSERT(ret == 0);
        }
    }
    if (flags & WriteFlag::File) {
        QMutexLocker locked(&s_mutexLogfile);
        // Writing to a closed QFile could cause an infinite recursive loop
        // by logging to qWarning!
        if (s_logfile.isOpen()) {
            const int written = s_logfile.write(message);
            Q_UNUSED(written);
            DEBUG_ASSERT(written == message.size());
            if (flags & WriteFlag::Flush) {
                const bool flushed = s_logfile.flush();
                Q_UNUSED(flushed);
                DEBUG_ASSERT(flushed);
            }
        }
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
    int baSize = 2 + 3 + 1; // all separators + newline (see below)
    const char* levelName = nullptr;
    WriteFlags writeFlags = WriteFlag::None;
    bool isDebugAssert = false;
    bool isControllerDebug = false;
    switch (type) {
    case QtDebugMsg:
        levelName = "Debug";
        baSize += strlen(levelName);
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
        baSize += strlen(levelName);
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
        baSize += strlen(levelName);
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
        baSize += strlen(levelName);
        writeFlags = WriteFlag::All;
        isDebugAssert = input.startsWith(QLatin1String(kDebugAssertPrefix));
        break;
    case QtFatalMsg:
        levelName = "Fatal";
        baSize += strlen(levelName);
        writeFlags = WriteFlag::All;
        break;
    }
    VERIFY_OR_DEBUG_ASSERT(levelName) {
        return;
    }

    QByteArray threadName =
            QThread::currentThread()->objectName().toLocal8Bit();
    if (threadName.isEmpty()) {
        QTextStream textStream(&threadName);
        textStream << QThread::currentThread();
    }
    baSize += threadName.size();

    QByteArray input8Bit;
    if (isControllerDebug) {
        input8Bit = input.mid(strlen(ControllerDebug::kLogMessagePrefix) + 1).toLocal8Bit();
    } else {
        input8Bit = input.toLocal8Bit();
    }
    baSize += input8Bit.size();

    const char* categoryName = context.category;
    int categoryName_len = 0;
    if (categoryName) {
        categoryName_len = strlen(categoryName);
        if (categoryName_len > 0) {
            if (strcmp(categoryName, kDefaultLoggingCategory.categoryName()) != 0) {
                baSize += 1; // additional separator (see below)
                baSize += categoryName_len;
            } else {
                // Suppress default category name
                categoryName = nullptr;
                categoryName_len = 0;
            }
        }
    }

    QByteArray ba;
    ba.reserve(baSize);

    ba.append(levelName);
    ba.append(" [");
    ba.append(threadName, threadName.size());
    if (categoryName) {
        ba.append("] ");
        ba.append(categoryName, categoryName_len);
        ba.append(": ");
    } else {
        ba.append("]: ");
    }
    ba.append(input8Bit, input8Bit.size());
    ba.append('\n');

    // Verify that the reserved size matches the actual size
    DEBUG_ASSERT(ba.size() == baSize);

    if (isDebugAssert) {
        if (s_debugAssertBreak) {
            writeToLog(ba, WriteFlag::All);
            raise(SIGINT);
            // When the debugger returns, continue normally.
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

    writeToLog(ba, writeFlags);
}

} // anonymous namespace

// Initialize the log level with the default value
LogLevel Logging::s_logLevel = kLogLevelDefault;
LogLevel Logging::s_logFlushLevel = kLogFlushLevelDefault;

// static
void Logging::initialize(const QDir& settingsDir,
        LogLevel logLevel,
        LogLevel logFlushLevel,
        bool debugAssertBreak) {
    VERIFY_OR_DEBUG_ASSERT(!s_logfile.isOpen()) {
        // Somebody already called Logging::initialize.
        return;
    }

    setLogLevel(logLevel);
    s_logFlushLevel = logFlushLevel;

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
            if (QFileInfo::exists(olderlogname)) {
                QFile::remove(olderlogname);
            }
            if (!QFile::rename(logFileName, olderlogname)) {
                fprintf(stderr, "Error rolling over logfile %s", logFileName.toLocal8Bit().constData());
            }
        }
    }

    // Since the message handler is not installed yet, we can touch s_logfile
    // without the lock.
    s_logfile.setFileName(logFileName);
    s_logfile.open(QIODevice::WriteOnly | QIODevice::Text);
    s_debugAssertBreak = debugAssertBreak;

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
