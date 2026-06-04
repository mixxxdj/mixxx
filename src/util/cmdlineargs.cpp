#include "util/cmdlineargs.h"

#include <qglobal.h>
#include <stdio.h>
#ifndef __WINDOWS__
#include <unistd.h>
#else
#include <io.h>
#endif

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QProcessEnvironment>
#include <QStandardPaths>
#include <QStyleFactory>
#include <QtGlobal>

#include "config.h"
#include "defs_urls.h"
#include "sources/soundsourceproxy.h"
#include "util/assert.h"

namespace {

bool calcUseColorsAuto() {
    // see https://no-color.org/
    if (QProcessEnvironment::systemEnvironment().contains(QLatin1String("NO_COLOR"))) {
        return false;
    }

#ifndef __WINDOWS__
    if (!isatty(fileno(stderr))) {
        return false;
    }
#else
    if (!_isatty(_fileno(stderr))) {
        return false;
    }
#endif

    // Check if terminal is known to support ANSI colors
    QString term = QProcessEnvironment::systemEnvironment().value("TERM");
    if (term == "alacritty" || term == "ansi" || term == "cygwin" || term == "linux" ||
            term.startsWith("screen") || term.startsWith("xterm") ||
            term.startsWith("vt100") || term.startsWith("rxvt") ||
            term.endsWith("color")) {
        return true;
    }
    return false;
}

bool parseLogLevel(
        const QString& logLevel,
        mixxx::LogLevel* pLogLevel) {
    if (logLevel.compare(QLatin1String("trace"), Qt::CaseInsensitive) == 0) {
        *pLogLevel = mixxx::LogLevel::Trace;
    } else if (logLevel.compare(QLatin1String("debug"), Qt::CaseInsensitive) == 0) {
        *pLogLevel = mixxx::LogLevel::Debug;
    } else if (logLevel.compare(QLatin1String("info"), Qt::CaseInsensitive) == 0) {
        *pLogLevel = mixxx::LogLevel::Info;
    } else if (logLevel.compare(QLatin1String("warning"), Qt::CaseInsensitive) == 0) {
        *pLogLevel = mixxx::LogLevel::Warning;
    } else if (logLevel.compare(QLatin1String("critical"), Qt::CaseInsensitive) == 0) {
        *pLogLevel = mixxx::LogLevel::Critical;
    } else {
        return false;
    }
    return true;
}

} // namespace

namespace mixxx {

CmdlineArgs::CmdlineArgs()
        : m_startInFullscreen(false), // Initialize vars
          m_startAutoDJ(false),
          m_rescanLibrary(false),
          m_controllerDebug(false),
          m_controllerPreviewScreens(false),
          m_controllerAbortOnWarning(false),
          m_developer(false),
          m_qml(false),
          m_awareOfRisk(false),
          m_safeMode(false),
          m_useLegacyVuMeter(false),
          m_useLegacySpinny(false),
          m_debugAssertBreak(false),
          m_settingsPathSet(false),
          m_scaleFactor(1.0),
          m_useColors(calcUseColorsAuto()),
          m_parseForUserFeedbackRequired(false),
          m_logLevel(mixxx::kLogLevelDefault),
          m_logFlushLevel(mixxx::kLogFlushLevelDefault),
          m_logMaxFileSize(mixxx::kLogMaxFileSizeDefault)
// We are not ready to switch to XDG folders under Linux, so keeping $HOME/.mixxx as preferences folder. see #8090
#if defined(__LINUX__) || defined(__BSD__)
#ifdef MIXXX_SETTINGS_PATH
          , m_settingsPath(QDir::homePath().append("/").append(MIXXX_SETTINGS_PATH))
#else
#error "We are not ready to switch to XDG folders under Linux"
#endif
#elif defined(Q_OS_IOS)
          // On iOS we intentionally use a user-accessible subdirectory of the sandbox
          // documents directory rather than the default app data directory. Specifically
          // we use
          //
          //     <sandbox home>/Documents/Library/Application Support/Mixxx
          //
          // instead of the default (and hidden)
          //
          //     <sandbox home>/Library/Application Support/Mixxx
          //
          // This lets the user back up their mixxxdb, add custom controller mappings,
          // potentially diagnose issues by accessing logs etc. via the native iOS files app.
          , m_settingsPath(
                  QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
                          .append("/Library/Application Support/Mixxx"))
#elif defined(Q_OS_ANDROID)
          // On Android we place settings (including logs) under the app's
          // user-visible "documents" folder:
          //
          //     /data/data/org.mixxx.Mixxx/files/documents/Mixxx
          //
          // The user confirmed they can browse this location with a file
          // manager, so logs (mixxx.log) and configuration are accessible
          // without needing adb or a logcat viewer.
          , m_settingsPath(
                  QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
                          .append("/Mixxx/"))
#else

          // TODO(XXX) Trailing slash not needed anymore as we switches from String::append
          // to QDir::filePath elsewhere in the code. This is candidate for removal.
          , m_settingsPath(
                  QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)
                          .append("/"))
#endif
{
}

bool CmdlineArgs::parse(int argc, char** argv) {
    QStringList arguments;
    for (int i = 0; i < argc; ++i) {
        arguments.append(QString::fromLocal8Bit(argv[i]));
    }
    return parse(arguments, ParseMode::Initial);
}

void CmdlineArgs::parseForUserFeedback() {
    if (!m_parseForUserFeedbackRequired) {
        return;
    }
    parse(QCoreApplication::arguments(), ParseMode::ForUserFeedback);
}

bool CmdlineArgs::parse(const QStringList& arguments, ParseMode mode) {
    bool forUserFeedback = (mode == ParseMode::ForUserFeedback);

    QCommandLineParser parser;

    if (forUserFeedback) {
        parser.setApplicationDescription(QCoreApplication::translate("CmdlineArgs",
                "Mixxx is an open-source digital DJ system. It's the professional-grade "
                "choice for DJs of all types and skill levels."));
    }

    const QCommandLineOption resourcePath(QStringLiteral("resourcePath"),
            forUserFeedback ? QCoreApplication::translate("CmdlineArgs",
                                      "The directory where Mixxx will look for its resource files.")
                            : QString(),
            QStringLiteral("directory"));
    QCommandLineOption resourcePathDeprecated(QStringLiteral("resource-path"),
            resourcePath.description(),
            resourcePath.valueName());
    parser.addOption(resourcePath);
    parser.addOption(resourcePathDeprecated);

    const QCommandLineOption settingsPath(QStringLiteral("settingsPath"),
            forUserFeedback ? QCoreApplication::translate("CmdlineArgs",
                                      "The directory where Mixxx will look for its "
                                      "user settings and database.")
                            : QString(),
            QStringLiteral("directory"));
    QCommandLineOption settingsPathDeprecated(QStringLiteral("settings-path"),
            settingsPath.description(),
            settingsPath.valueName());
    parser.addOption(settingsPath);
    parser.addOption(settingsPathDeprecated);

    const QCommandLineOption timelinePath(QStringLiteral("timelinePath"),
            forUserFeedback ? QCoreApplication::translate("CmdlineArgs",
                                      "The directory where Mixxx will look for its stats and usage info.")
                            : QString(),
            QStringLiteral("directory"));
    QCommandLineOption timelinePathDeprecated(QStringLiteral("timeline-path"),
            timelinePath.description(),
            timelinePath.valueName());
    parser.addOption(timelinePath);
    parser.addOption(timelinePathDeprecated);

    const QCommandLineOption fullScreen(QStringLiteral("f"),
            forUserFeedback ? QCoreApplication::translate("CmdlineArgs", "Starts Mixxx in fullscreen.")
                            : QString());
    QCommandLineOption fullScreenDeprecated(QStringLiteral("fullscreen"), fullScreen.description());
    parser.addOption(fullScreen);
    parser.addOption(fullScreenDeprecated);

    const QCommandLineOption startAutoDJ(QStringLiteral("autodj"),
            forUserFeedback ? QCoreApplication::translate("CmdlineArgs", "Starts AutoDJ on start-up.")
                            : QString());
    parser.addOption(startAutoDJ);

    const QCommandLineOption rescanLibrary(QStringLiteral("rescan-library"),
            forUserFeedback ? QCoreApplication::translate("CmdlineArgs", "Rescans the library on start-up.")
                            : QString());
    parser.addOption(rescanLibrary);

    const QCommandLineOption enableLegacyVuMeter(QStringLiteral("enable-legacy-vu-meter"),
            forUserFeedback ? QCoreApplication::translate("CmdlineArgs",
                                      "Enables the legacy VU meter behavior.")
                            : QString());
    parser.addOption(enableLegacyVuMeter);

    const QCommandLineOption enableLegacySpinny(QStringLiteral("enable-legacy-spinny"),
            forUserFeedback ? QCoreApplication::translate("CmdlineArgs",
                                      "Enables the legacy spinny behavior.")
                            : QString());
    parser.addOption(enableLegacySpinny);

    const QCommandLineOption controllerDebug(QStringLiteral("controller-debug"),
            forUserFeedback ? QCoreApplication::translate("CmdlineArgs",
                                      "Enables extra debugging info for controllers.")
                            : QString());
    QCommandLineOption controllerDebugDeprecated(QStringLiteral("controllerDebug"),
            controllerDebug.description());
    controllerDebugDeprecated.setFlags(QCommandLineOption::HiddenFromHelp);
    parser.addOption(controllerDebug);
    parser.addOption(controllerDebugDeprecated);

    const QCommandLineOption controllerAbortOnWarning(QStringLiteral("controller-abort-on-warning"),
            forUserFeedback ? QCoreApplication::translate("CmdlineArgs",
                                      "Makes the controller engine abort on warnings.")
                            : QString());
    parser.addOption(controllerAbortOnWarning);

    const QCommandLineOption locale(QStringLiteral("locale"),
            forUserFeedback ? QCoreApplication::translate("CmdlineArgs",
                                      "Forces Mixxx to use the specified locale.")
                            : QString(),
            QStringLiteral("locale"));
    parser.addOption(locale);

    const QCommandLineOption color(QStringLiteral("color"),
            forUserFeedback ? QCoreApplication::translate("CmdlineArgs",
                                      "Sets if colors should be used in the console output. "
                                      "Possible values: always, never, auto. The default is auto.")
                            : QString(),
            QStringLiteral("mode"),
            QStringLiteral("auto"));
    parser.addOption(color);

    const QCommandLineOption safeMode(QStringLiteral("safe-mode"),
            forUserFeedback ? QCoreApplication::translate("CmdlineArgs",
                                      "Starts Mixxx in safe-mode (disables OpenGL, "
                                      "hardware acceleration, etc.).")
                            : QString());
    QCommandLineOption safeModeDeprecated(QStringLiteral("safeMode"), safeMode.description());
    safeModeDeprecated.setFlags(QCommandLineOption::HiddenFromHelp);
    parser.addOption(safeMode);
    parser.addOption(safeModeDeprecated);

    const QCommandLineOption developer(QStringLiteral("developer"),
            forUserFeedback ? QCoreApplication::translate("CmdlineArgs",
                                      "Enables developer-mode. Includes extra log info, stats on "
                                      "performance, and a Developer tools menu.")
                            : QString());
    parser.addOption(developer);

    const QCommandLineOption qml(QStringLiteral("new-ui"),
            forUserFeedback
                    ? QCoreApplication::translate("CmdlineArgs",
                              "Loads the highly unstable 3.0 Mixxx interface, "
                              "based on QML. You need to use a new setting "
                              "profile, or run with "
                              "'allow-dangerous-data-corruption-risk' to use "
                              "with the current one. We highly recommend "
                              "backing up your data if you do so.")
                    : QString());
    QCommandLineOption qmlDeprecated(
            QStringLiteral("qml"));
    qmlDeprecated.setFlags(QCommandLineOption::HiddenFromHelp);
    parser.addOption(qmlDeprecated);
    parser.addOption(qml);
    const QCommandLineOption awareOfRisk(
            QStringLiteral("allow-dangerous-data-corruption-risk"),
            forUserFeedback
                    ? QCoreApplication::translate("CmdlineArgs",
                              "Force Mixxx to load an unstable version with an "
                              "existing user profile from a stable version")
                    : QString());
    parser.addOption(awareOfRisk);

    const QCommandLineOption logLevel(QStringLiteral("log-level"),
            forUserFeedback ? QCoreApplication::translate("CmdlineArgs",
                                      "Sets the verbosity of the console output. "
                                      "<level> is one of:\n"
                                      "critical - Critical/Fatal only\n"
                                      "warning  - Above + Warnings\n"
                                      "info     - Above + Informational messages\n"
                                      "debug    - Above + Debug/Developer messages\n"
                                      "trace    - Above + Profiling messages")
                            : QString(),
            QStringLiteral("level"));
    QCommandLineOption logLevelDeprecated(QStringLiteral("logLevel"), logLevel.description());
    logLevelDeprecated.setFlags(QCommandLineOption::HiddenFromHelp);
    logLevelDeprecated.setValueName(logLevel.valueName());
    parser.addOption(logLevel);
    parser.addOption(logLevelDeprecated);

    const QCommandLineOption logFlushLevel(QStringLiteral("log-flush-level"),
            forUserFeedback ? QCoreApplication::translate("CmdlineArgs",
                                      "Sets the the logging level at which the log buffer is "
                                      "flushed to mixxx.log. <level> is one of the values defined "
                                      "at --log-level above.")
                            : QString(),
            QStringLiteral("level"));
    QCommandLineOption logFlushLevelDeprecated(
            QStringLiteral("logFlushLevel"), logLevel.description());
    logFlushLevelDeprecated.setFlags(QCommandLineOption::HiddenFromHelp);
    logFlushLevelDeprecated.setValueName(logFlushLevel.valueName());
    parser.addOption(logFlushLevel);
    parser.addOption(logFlushLevelDeprecated);

    const QCommandLineOption logMaxFileSize(QStringLiteral("log-max-file-size"),
            forUserFeedback ? QCoreApplication::translate("CmdlineArgs",
                                      "Sets the maximum file size of the "
                                      "mixxx.log file in bytes. "
                                      "Use -1 for unlimited. The default is "
                                      "100 MB as 1e5 or 100000000.")
                            : QString(),
            QStringLiteral("bytes"));
    parser.addOption(logMaxFileSize);

    QCommandLineOption debugAssertBreak(QStringLiteral("debug-assert-break"),
            forUserFeedback ? QCoreApplication::translate("CmdlineArgs",
                                      "Breaks (SIGINT) Mixxx, if a DEBUG_ASSERT evaluates to "
                                      "false. Under a debugger you can continue afterwards.")
                            : QString());
    QCommandLineOption debugAssertBreakDeprecated(
            QStringLiteral("debugAssertBreak"), debugAssertBreak.description());
    debugAssertBreakDeprecated.setFlags(QCommandLineOption::HiddenFromHelp);
    parser.addOption(debugAssertBreak);
    parser.addOption(debugAssertBreakDeprecated);

    const QCommandLineOption styleOption(QStringLiteral("style"),
            forUserFeedback
                    ? QCoreApplication::translate("CmdlineArgs",
                              "Overrides the default application GUI style. Possible values: %1")
                              .arg(QStyleFactory::keys().join(QStringLiteral(", ")))
                    : QString(),
            QStringLiteral("style"));
    parser.addOption(styleOption);

    const QCommandLineOption helpOption = parser.addHelpOption();
    const QCommandLineOption versionOption = parser.addVersionOption();

    parser.addPositionalArgument(QStringLiteral("file"),
            forUserFeedback ? QCoreApplication::translate("CmdlineArgs",
                                      "Load the specified music file(s) at start-up. Each file "
                                      "you specify will be loaded into the next virtual deck.")
                            : QString());

    const QCommandLineOption controllerPreviewScreens(QStringLiteral("controller-preview-screens"),
            forUserFeedback ? QCoreApplication::translate("CmdlineArgs",
                                      "Preview rendered controller screens in the Setting windows.")
                            : QString());
    parser.addOption(controllerPreviewScreens);

    if (forUserFeedback) {
        // We know form the first path, that there will be likely an error message, check again.
        // This is not the case if the user uses a Qt internal option that is unknown
        // in the first path
        puts(""); // Add a blank line to make the parser output more visible
                  // This call does not return and calls exit() in case of help or an parser error
        parser.process(arguments);
        return true;
    }

    // From here, we are in in the initial parse mode
    DEBUG_ASSERT(mode == ParseMode::Initial);

    // process all arguments
    if (!parser.parse(arguments)) {
        // we have an misspelled argument or one that is processed
        // in the not yet initialized QCoreApplication
        m_parseForUserFeedbackRequired = true;
    }

    if (parser.isSet(versionOption) ||
            parser.isSet(helpOption)
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
            || parser.isSet(QStringLiteral("help-all"))
#endif
    ) {
        m_parseForUserFeedbackRequired = true;
    }

    m_startInFullscreen = parser.isSet(fullScreen) || parser.isSet(fullScreenDeprecated);

    if (parser.isSet(locale)) {
        m_locale = parser.value(locale);
    }

    if (parser.isSet(startAutoDJ)) {
        m_startAutoDJ = true;
    }

    if (parser.isSet(rescanLibrary)) {
        m_rescanLibrary = true;
    }

    if (parser.isSet(settingsPath)) {
        m_settingsPath = parser.value(settingsPath);
        if (!m_settingsPath.endsWith("/")) {
            m_settingsPath.append("/");
        }
        m_settingsPathSet = true;
    } else if (parser.isSet(settingsPathDeprecated)) {
        m_settingsPath = parser.value(settingsPathDeprecated);
        if (!m_settingsPath.endsWith("/")) {
            m_settingsPath.append("/");
        }
        m_settingsPathSet = true;
    }

    if (parser.isSet(resourcePath)) {
        m_resourcePath = parser.value(resourcePath);
    } else if (parser.isSet(resourcePathDeprecated)) {
        m_resourcePath = parser.value(resourcePathDeprecated);
    }

    if (parser.isSet(timelinePath)) {
        m_timelinePath = parser.value(timelinePath);
    } else if (parser.isSet(timelinePathDeprecated)) {
        m_timelinePath = parser.value(timelinePathDeprecated);
    }

    m_useLegacyVuMeter = parser.isSet(enableLegacyVuMeter);
    m_useLegacySpinny = parser.isSet(enableLegacySpinny);
    m_controllerDebug = parser.isSet(controllerDebug) || parser.isSet(controllerDebugDeprecated);
    m_controllerPreviewScreens = parser.isSet(controllerPreviewScreens);
    m_controllerAbortOnWarning = parser.isSet(controllerAbortOnWarning);
    m_developer = parser.isSet(developer);

    m_qml = parser.isSet(qml);
    if (parser.isSet(qmlDeprecated)) {
        m_qml |= true;
        qWarning() << "The argument '--qml' is deprecated and will be soon "
                      "removed. Please use '--new-ui' instead!";
    }
    m_awareOfRisk = parser.isSet(awareOfRisk);

    m_safeMode = parser.isSet(safeMode) || parser.isSet(safeModeDeprecated);
    m_debugAssertBreak = parser.isSet(debugAssertBreak) || parser.isSet(debugAssertBreakDeprecated);

    m_musicFiles = parser.positionalArguments();

    if (parser.isSet(logLevel)) {
        if (!parseLogLevel(parser.value(logLevel), &m_logLevel)) {
            fputs("\nlog-level wasn't 'trace', 'debug', 'info', 'warning', or 'critical'!\n"
                  "Mixxx will only print warnings and critical messages to the console.\n",
                    stdout);
        }
    } else if (parser.isSet(logLevelDeprecated)) {
        if (!parseLogLevel(parser.value(logLevelDeprecated), &m_logLevel)) {
            fputs("\nlogLevel wasn't 'trace', 'debug', 'info', 'warning', or 'critical'!\n"
                  "Mixxx will only print warnings and critical messages to the console.\n",
                    stdout);
        }
    } else {
        if (m_developer) {
            m_logLevel = mixxx::LogLevel::Debug;
        }
    }

    if (parser.isSet(logFlushLevel)) {
        if (!parseLogLevel(parser.value(logFlushLevel), &m_logFlushLevel)) {
            fputs("\nlog-flush-level wasn't 'trace', 'debug', 'info', 'warning', or 'critical'!\n"
                  "Mixxx will only flush output after a critical message.\n",
                    stdout);
        }
    } else if (parser.isSet(logFlushLevelDeprecated)) {
        if (!parseLogLevel(parser.value(logFlushLevelDeprecated), &m_logFlushLevel)) {
            fputs("\nlogFlushLevel wasn't 'trace', 'debug', 'info', 'warning', or 'critical'!\n"
                  "Mixxx will only flush output after a critical message.\n",
                    stdout);
        }
    }

    if (parser.isSet(logMaxFileSize)) {
        QString strLogMaxFileSize = parser.value(logMaxFileSize);
        bool ok = false;
        // We parse it as double to also support exponential notation
        m_logMaxFileSize = static_cast<qint64>(strLogMaxFileSize.toDouble(&ok));
        if (!ok) {
            fputs("\nFailed to parse log-max-file-size.\n", stdout);
            return false;
        }
    }

    // set colors
    if (parser.value(color).compare(QLatin1String("always"), Qt::CaseInsensitive) == 0) {
        m_useColors = true;
    } else if (parser.value(color).compare(QLatin1String("never"), Qt::CaseInsensitive) == 0) {
        m_useColors = false;
    } else if (parser.value(color).compare(QLatin1String("auto"), Qt::CaseInsensitive) != 0) {
        fputs("Unknown argument for for color.\n", stdout);
    }

    if (parser.isSet(styleOption)) {
        m_styleName = parser.value(styleOption);
    }

    return true;
}

} // namespace mixxx
