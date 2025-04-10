#include "util/cmdlineargs.h"

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
    return term == "alacritty" || term == "ansi" || term == "cygwin" || term == "linux" ||
            term.startsWith("screen") || term.startsWith("xterm") ||
            term.startsWith("vt100") || term.startsWith("rxvt") ||
            term.endsWith("color");
}

} // namespace

CmdlineArgs::CmdlineArgs()
        : m_startInFullscreen(false), // Initialize vars
          m_startAutoDJ(false),
          m_rescanLibrary(false),
          m_controllerDebug(false),
          m_controllerAbortOnWarning(false),
          m_developer(false),
#ifdef MIXXX_USE_QML
          m_qml(false),
#endif
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
          m_logMaxFileSize(mixxx::kLogMaxFileSizeDefault),
// We are not ready to switch to XDG folders under Linux, so keeping $HOME/.mixxx as preferences folder. see #8090
#ifdef MIXXX_SETTINGS_PATH
          m_settingsPath(QDir::homePath().append("/").append(MIXXX_SETTINGS_PATH))
#elif defined(__LINUX__)
#error "We are not ready to switch to XDG folders under Linux"
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
          m_settingsPath(
                  QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
                          .append("/Library/Application Support/Mixxx"))
#else

          // TODO(XXX) Trailing slash not needed anymore as we switches from String::append
          // to QDir::filePath elsewhere in the code. This is candidate for removal.
          m_settingsPath(
                  QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)
                          .append("/"))
#endif
{
}

namespace {
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

bool CmdlineArgs::parse(int argc, char** argv) {
    // Some command line parameters needs to be evaluated before
    // The QCoreApplication is initialized.
    DEBUG_ASSERT(!QCoreApplication::instance());
    if (argc == 1) {
        // Mixxx was run with the binary name only, nothing to do
        return true;
    }
    QStringList arguments;
    arguments.reserve(argc);
    for (int a = 0; a < argc; ++a) {
        arguments << QString::fromLocal8Bit(argv[a]);
    }
    return parse(arguments, ParseMode::Initial);
}

void CmdlineArgs::parseForUserFeedback() {
    // For user feedback we need an initialized QCoreApplication because
    // it add some QT specific command line parameters
    DEBUG_ASSERT(QCoreApplication::instance());
    // We need only execute the second parse for user feedback when the first run
    // has produces an not yet displayed error or help text.
    // Otherwise we can skip the second run.
    // A parameter for the QCoreApplication will fail in the first run and
    // m_parseForUserFeedbackRequired will be set as well.
    if (!m_parseForUserFeedbackRequired) {
        return;
    }
    parse(QCoreApplication::arguments(), ParseMode::ForUserFeedback);
}

bool CmdlineArgs::parse(const QStringList& arguments, CmdlineArgs::ParseMode mode) {
    bool forUserFeedback = (mode == ParseMode::ForUserFeedback);

    QCommandLineParser parser;
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);

    if (forUserFeedback) {
        parser.setApplicationDescription(
                QCoreApplication::translate("CmdlineArgs",
                        "Mixxx is an open source DJ software. For more "
                        "information, see: ") +
                MIXXX_MANUAL_COMMANDLINEOPTIONS_URL);
    }
    // add options
    const QCommandLineOption fullScreen(
            QStringList({QStringLiteral("f"), QStringLiteral("full-screen")}),
            forUserFeedback ? QCoreApplication::translate(
                                      "CmdlineArgs", "Starts Mixxx in full-screen mode")
                            : QString());
    QCommandLineOption fullScreenDeprecated(QStringLiteral("fullScreen"));
    fullScreenDeprecated.setFlags(QCommandLineOption::HiddenFromHelp);
    parser.addOption(fullScreen);
    parser.addOption(fullScreenDeprecated);

    const QCommandLineOption locale(QStringLiteral("locale"),
            forUserFeedback ? QCoreApplication::translate("CmdlineArgs",
                                      "Use a custom locale for loading translations. (e.g "
                                      "'fr')")
                            : QString(),
            QStringLiteral("locale"));
    parser.addOption(locale);

    const QCommandLineOption startAutoDJ(QStringLiteral("start-autodj"),
            forUserFeedback ? QCoreApplication::translate("CmdlineArgs",
                                      "Starts Auto DJ when Mixxx is launched.")
                            : QString());
    parser.addOption(startAutoDJ);

    const QCommandLineOption rescanLibrary(QStringLiteral("rescan-library"),
            forUserFeedback ? QCoreApplication::translate("CmdlineArgs",
                                      "Rescans the library when Mixxx is launched.")
                            : QString());
    parser.addOption(rescanLibrary);

    // An option with a value
    const QCommandLineOption settingsPath(QStringLiteral("settings-path"),
            forUserFeedback ? QCoreApplication::translate("CmdlineArgs",
                                      "Top-level directory where Mixxx should look for settings. "
                                      "Default is: ") +
                            QDir::toNativeSeparators(getSettingsPath())
                            : QString(),
            QStringLiteral("path"));
    QCommandLineOption settingsPathDeprecated(
            QStringLiteral("settingsPath"));
    settingsPathDeprecated.setFlags(QCommandLineOption::HiddenFromHelp);
    settingsPathDeprecated.setValueName(settingsPath.valueName());
    parser.addOption(settingsPath);
    parser.addOption(settingsPathDeprecated);

    QCommandLineOption resourcePath(QStringLiteral("resource-path"),
            forUserFeedback ? QCoreApplication::translate("CmdlineArgs",
                                      "Top-level directory where Mixxx should look for its "
                                      "resource files such as MIDI mappings, overriding the "
                                      "default installation location.")
                            : QString(),
            QStringLiteral("path"));
    QCommandLineOption resourcePathDeprecated(
            QStringLiteral("resourcePath"));
    resourcePathDeprecated.setFlags(QCommandLineOption::HiddenFromHelp);
    resourcePathDeprecated.setValueName(resourcePath.valueName());
    parser.addOption(resourcePath);
    parser.addOption(resourcePathDeprecated);

    const QCommandLineOption timelinePath(QStringLiteral("timeline-path"),
            forUserFeedback ? QCoreApplication::translate("CmdlineArgs",
                                      "Path the debug statistics time line is written to")
                            : QString(),
            QStringLiteral("path"));
    QCommandLineOption timelinePathDeprecated(
            QStringLiteral("timelinePath"), timelinePath.description());
    timelinePathDeprecated.setFlags(QCommandLineOption::HiddenFromHelp);
    timelinePathDeprecated.setValueName(timelinePath.valueName());
    parser.addOption(timelinePath);
    parser.addOption(timelinePathDeprecated);

    const QCommandLineOption enableLegacyVuMeter(QStringLiteral("enable-legacy-vumeter"),
            forUserFeedback ? QCoreApplication::translate("CmdlineArgs",
                                      "Use legacy vu meter")
                            : QString());
    parser.addOption(enableLegacyVuMeter);

    const QCommandLineOption enableLegacySpinny(QStringLiteral("enable-legacy-spinny"),
            forUserFeedback ? QCoreApplication::translate("CmdlineArgs",
                                      "Use legacy spinny")
                            : QString());
    parser.addOption(enableLegacySpinny);

    const QCommandLineOption controllerDebug(QStringLiteral("controller-debug"),
            forUserFeedback ? QCoreApplication::translate("CmdlineArgs",
                                      "Causes Mixxx to display/log all of the controller data it "
                                      "receives and script functions it loads")
                            : QString());
    QCommandLineOption controllerDebugDeprecated(
            QStringList({QStringLiteral("controllerDebug"),
                    QStringLiteral("midiDebug")}));
    controllerDebugDeprecated.setFlags(QCommandLineOption::HiddenFromHelp);
    parser.addOption(controllerDebug);
    parser.addOption(controllerDebugDeprecated);

    const QCommandLineOption controllerAbortOnWarning(
            QStringLiteral("controller-abort-on-warning"),
            forUserFeedback ? QCoreApplication::translate("CmdlineArgs",
                                      "The controller mapping will issue more "
                                      "aggressive warnings and errors when "
                                      "detecting misuse of controller APIs. "
                                      "New Controller Mappings should be "
                                      "developed with this option enabled!")
                            : QString());
    parser.addOption(controllerAbortOnWarning);

    const QCommandLineOption developer(QStringLiteral("developer"),
            forUserFeedback ? QCoreApplication::translate("CmdlineArgs",
                                      "Enables developer-mode. Includes extra log info, stats on "
                                      "performance, and a Developer tools menu.")
                            : QString());
    parser.addOption(developer);

#ifdef MIXXX_USE_QML
    const QCommandLineOption qml(QStringLiteral("qml"),
            forUserFeedback ? QCoreApplication::translate("CmdlineArgs",
                                      "Loads experimental QML GUI instead of legacy QWidget skin")
                            : QString());
    parser.addOption(qml);
#endif
    const QCommandLineOption safeMode(QStringLiteral("safe-mode"),
            forUserFeedback ? QCoreApplication::translate("CmdlineArgs",
                                      "Enables safe-mode. Disables OpenGL waveforms, and "
                                      "spinning vinyl widgets. Try this option if Mixxx is "
                                      "crashing on startup.")
                            : QString());
    QCommandLineOption safeModeDeprecated(QStringLiteral("safeMode"), safeMode.description());
    safeModeDeprecated.setFlags(QCommandLineOption::HiddenFromHelp);
    parser.addOption(safeMode);
    parser.addOption(safeModeDeprecated);

    const QCommandLineOption color(QStringLiteral("color"),
            forUserFeedback ? QCoreApplication::translate("CmdlineArgs",
                                      "[auto|always|never] Use colors on the console output.")
                            : QString(),
            QStringLiteral("color"),
            QStringLiteral("auto"));
    parser.addOption(color);

    const QCommandLineOption logLevel(QStringLiteral("log-level"),
            forUserFeedback ? QCoreApplication::translate("CmdlineArgs",
                                      "Sets the verbosity of command line logging.\n"
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
    logFlushLevelDeprecated.setFlags(QCommandLineOption::HiddenFromHelp);
    logFlushLevelDeprecated.setValueName(logFlushLevel.valueName());
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
#ifdef MIXXX_USE_QML
    m_qml = parser.isSet(qml);
#endif
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
