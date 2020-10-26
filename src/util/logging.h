#pragma once

#include <QDir>
#include <QLoggingCategory>

/// Enable/disable verbose debug logging globally.
///
/// Verbose debug logging is only needed temporarily during
/// development and will probably cause performance degressions,
/// especially when debug messages are logged from real-time code.
/// IT SHOULD BE KEPT DISABLED IN REGULAR BUILDS AND IS ONLY
/// SUITABLE FOR DEVELOPMENT!
///
/// After switching the default log level from Debug to Info
/// this #define might be enabled permanently and could then
/// be removed from the code. Debug logs are intended for
/// developers, not for regular users.
///
/// Example for verbose debug logs: SoundSourceFFmpeg
#define VERBOSE_DEBUG_LOG false

/// Macro for composing parent/child logging category names
///
/// Use this macro to compose a new child logging category from a
/// parent category.
#define MIXXX_LOGGING_CATEGORY_PARENT_CHILD(parentName, childName) \
    parentName "." childName

/// Root logging category for Mixxx
///
/// Only use this as a parent if none of the predefined base logging
/// categories (see below) are suitable. In this case discuss with the
/// team if a new base category needs to be introduced.
Q_DECLARE_LOGGING_CATEGORY(mixxxLog)
#define MIXXX_LOGGING_CATEGORY_ROOT "mixxx"

/// Base logging category for configuration
Q_DECLARE_LOGGING_CATEGORY(mixxxLogConfig)
#define MIXXX_LOGGING_CATEGORY_CONFIG \
    MIXXX_LOGGING_CATEGORY_PARENT_CHILD(MIXXX_LOGGING_CATEGORY_ROOT, "config")

/// Base logging category for controls and controllers
Q_DECLARE_LOGGING_CATEGORY(mixxxLogControl)
#define MIXXX_LOGGING_CATEGORY_CONTROL \
    MIXXX_LOGGING_CATEGORY_PARENT_CHILD(MIXXX_LOGGING_CATEGORY_ROOT, "control")

/// Base logging category for generic database access
Q_DECLARE_LOGGING_CATEGORY(mixxxLogDatabase)
#define MIXXX_LOGGING_CATEGORY_DATABASE \
    MIXXX_LOGGING_CATEGORY_PARENT_CHILD(MIXXX_LOGGING_CATEGORY_ROOT, "database")

/// Base logging category for hardware I/O sound devices
Q_DECLARE_LOGGING_CATEGORY(mixxxLogDevice)
#define MIXXX_LOGGING_CATEGORY_DEVICE \
    MIXXX_LOGGING_CATEGORY_PARENT_CHILD(MIXXX_LOGGING_CATEGORY_ROOT, "device")

/// Base logging category for audio processing (not real-time safe!)
Q_DECLARE_LOGGING_CATEGORY(mixxxLogEngine)
#define MIXXX_LOGGING_CATEGORY_ENGINE \
    MIXXX_LOGGING_CATEGORY_PARENT_CHILD(MIXXX_LOGGING_CATEGORY_ROOT, "engine")

/// Base logging category for library management
Q_DECLARE_LOGGING_CATEGORY(mixxxLogLibrary)
#define MIXXX_LOGGING_CATEGORY_LIBRARY \
    MIXXX_LOGGING_CATEGORY_PARENT_CHILD(MIXXX_LOGGING_CATEGORY_ROOT, "library")

/// Base logging category for generic network communication
Q_DECLARE_LOGGING_CATEGORY(mixxxLogNetwork)
#define MIXXX_LOGGING_CATEGORY_NETWORK \
    MIXXX_LOGGING_CATEGORY_PARENT_CHILD(MIXXX_LOGGING_CATEGORY_ROOT, "network")

/// Base logging category for scripting
Q_DECLARE_LOGGING_CATEGORY(mixxxLogScript)
#define MIXXX_LOGGING_CATEGORY_SCRIPT \
    MIXXX_LOGGING_CATEGORY_PARENT_CHILD(MIXXX_LOGGING_CATEGORY_ROOT, "script")

/// Base logging category for audio and metadata sources
Q_DECLARE_LOGGING_CATEGORY(mixxxLogSource)
#define MIXXX_LOGGING_CATEGORY_SOURCE \
    MIXXX_LOGGING_CATEGORY_PARENT_CHILD(MIXXX_LOGGING_CATEGORY_ROOT, "source")

namespace mixxx {

/// Custom enumeration that contains a subset of QtMsgType
///
/// Uses a reverse ordering compared to QtMsgType, i.e. the
/// most severe level has the lowest number!
///
/// TODO: Reverse the ordering and use the corresponding QtMsgType
/// ordinals for the numbering after `Trace` has been removed. Check
/// for ALL occurences that might be affected by this reordering!!
enum class LogLevel {
    Critical = 0,
    Warning = 1,
    Info = 2,
    Debug = 3,
    Trace = 4, // DEPRECATED (not available in Qt, used for profiling etc.)
};

/// Default log level for (console) logs.
constexpr LogLevel kLogLevelDefault = LogLevel::Warning;

/// Default log level for flushing the buffered log stream.
/// This is required to ensure that all buffered messages have
/// been written before Mixxx crashes.
constexpr LogLevel kLogFlushLevelDefault = LogLevel::Critical;

/// Utility class for accessing the logging settings that are configured
/// at startup.
///
/// Do not use this class directly!! Instead use the following Qt functions
/// (= macros) for logging:
///
///   - qCCritical(): Unexpected errors that might not be recoverable,
///                   e.g. failed database queries (HTTP Status 5xx)
///   - qCWarning(): Expected errors that could be handled and are
///                  recoverable, e.g. invalid or inconsistent input data
///                  (HTTP Status 4xx)
///   - qCInfo(): Everything that should appear in the log file and that
///               could help to analyze and identify issues that might occur
///   - qCDebug(): Information that is only relevant during development
///
/// If you need more detailed trace logs during debugging or implementing
/// new functionality then hide this code behind `#define ENABLE_TRACE_LOG false`
/// in the corresponding compilation unit (= .cpp file). Enable it temporarily
/// in your local repo while debugging. Disable it before submitting a pull
/// request. Use qCDebug() for these trace logs.
///
/// Examples:
///   - sources/soundsourceffmpeg.h/.cpp
///   - soundio/sounddeviceportaudio.h/.cpp
///
/// See also: https://doc.qt.io/qt-5/qloggingcategory.html
class Logging {
  public:
    // These are not thread safe. Only call them on Mixxx startup and shutdown.
    static void initialize(
            const QDir& logDir,
            LogLevel logLevel,
            LogLevel logFlushLevel,
            bool debugAssertBreak);

    // Sets only the loglevel without the on-disk settings. Used by mixxx-test.
    static void setLogLevel(
            LogLevel logLevel) {
        s_logLevel = logLevel;
    }

    static void shutdown();

    static void flushLogFile();

    static bool shouldFlush(
            LogLevel logFlushLevel) {
        // Log levels are ordered by severity, i.e. more
        // severe log levels have a lower ordinal
        return s_logFlushLevel >= logFlushLevel;
    }

    static bool enabled(
            LogLevel logLevel) {
        return s_logLevel >= logLevel;
    }

  private:
    // Almost constant, i.e. initialized once at startup and
    // then could safely be read from multiple threads.
    static LogLevel s_logLevel;
    static LogLevel s_logFlushLevel;

    Logging() = delete;
};

} // namespace mixxx
