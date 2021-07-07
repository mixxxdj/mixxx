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

#include "config.h"
#include "defs_urls.h"
#include "sources/soundsourceproxy.h"

CmdlineArgs::CmdlineArgs()
        : m_startInFullscreen(false), // Initialize vars
          m_midiDebug(false),
          m_developer(false),
          m_safeMode(false),
          m_debugAssertBreak(false),
          m_settingsPathSet(false),
          m_useColors(false),
          m_logLevel(mixxx::kLogLevelDefault),
          m_logFlushLevel(mixxx::kLogFlushLevelDefault),
// We are not ready to switch to XDG folders under Linux, so keeping $HOME/.mixxx as preferences folder. see lp:1463273
#ifdef MIXXX_SETTINGS_PATH
          m_settingsPath(QDir::homePath().append("/").append(MIXXX_SETTINGS_PATH)) {
#else
#ifdef __LINUX__
#error "We are not ready to switch to XDG folders under Linux"
#endif

          // TODO(XXX) Trailing slash not needed anymore as we switches from String::append
          // to QDir::filePath elsewhere in the code. This is candidate for removal.
          m_settingsPath(
                  QStandardPaths::writableLocation(QStandardPaths::DataLocation)
                          .append("/")) {
#endif
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
    if (argc == 1) {
        // Mixxx was run with the binary name only, nothing to do
        return true;
    }
    QStringList arguments;
    arguments.reserve(argc);
    for (int a = 0; a < argc; ++a) {
        arguments << QString::fromLocal8Bit(argv[a]);
    }

    QCommandLineParser parser;
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);

    parser.setApplicationDescription(
            QCoreApplication::translate("CmdlineArgs",
                    "Mixxx is an open source DJ software. For more "
                    "information, see: ") +
            MIXXX_MANUAL_COMMANDLINEOPTIONS_URL);

    // add options
    const QCommandLineOption fullScreen(
            QStringList({QStringLiteral("f"), QStringLiteral("full-screen")}),
            QCoreApplication::translate(
                    "CmdlineArgs", "Starts Mixxx in full-screen mode"));
    QCommandLineOption fullScreenDeprecated(QStringLiteral("fullScreen"));
    fullScreenDeprecated.setFlags(QCommandLineOption::HiddenFromHelp);
    parser.addOption(fullScreen);
    parser.addOption(fullScreenDeprecated);

    const QCommandLineOption locale(QStringLiteral("locale"),
            QCoreApplication::translate("CmdlineArgs",
                    "Use a custom locale for loading translations. (e.g "
                    "'fr')"),
            QStringLiteral("locale"));
    parser.addOption(locale);

    // An option with a value
    const QCommandLineOption settingsPath(QStringLiteral("settings-path"),
            QCoreApplication::translate("CmdlineArgs",
                    "Top-level directory where Mixxx should look for settings. "
                    "Default is:") +
                    getSettingsPath(),
            QStringLiteral("path"));
    QCommandLineOption settingsPathDeprecated(
            QStringLiteral("settingsPath"));
    settingsPathDeprecated.setFlags(QCommandLineOption::HiddenFromHelp);
    settingsPathDeprecated.setValueName(settingsPath.valueName());
    parser.addOption(settingsPath);
    parser.addOption(settingsPathDeprecated);

    QCommandLineOption resourcePath(QStringLiteral("resource-path"),
            QCoreApplication::translate("CmdlineArgs",
                    "Top-level directory where Mixxx should look for its "
                    "resource files such as MIDI mappings, overriding the "
                    "default installation location."),
            QStringLiteral("path"));
    QCommandLineOption resourcePathDeprecated(
            QStringLiteral("resourcePath"));
    resourcePathDeprecated.setFlags(QCommandLineOption::HiddenFromHelp);
    resourcePathDeprecated.setValueName(resourcePath.valueName());
    parser.addOption(resourcePath);
    parser.addOption(resourcePathDeprecated);

    const QCommandLineOption timelinePath(QStringLiteral("timeline-path"),
            QCoreApplication::translate("CmdlineArgs",
                    "Path the debug statistics time line is written to"),
            QStringLiteral("path"));
    QCommandLineOption timelinePathDeprecated(
            QStringLiteral("timelinePath"), timelinePath.description());
    timelinePathDeprecated.setFlags(QCommandLineOption::HiddenFromHelp);
    timelinePathDeprecated.setValueName(timelinePath.valueName());
    parser.addOption(timelinePath);
    parser.addOption(timelinePathDeprecated);

    const QCommandLineOption controllerDebug(QStringLiteral("controller-debug"),
            QCoreApplication::translate("CmdlineArgs",
                    "Causes Mixxx to display/log all of the controller data it "
                    "receives and script functions it loads"));
    QCommandLineOption controllerDebugDeprecated(
            QStringList({QStringLiteral("controllerDebug"),
                    QStringLiteral("midiDebug")}));
    controllerDebugDeprecated.setFlags(QCommandLineOption::HiddenFromHelp);
    parser.addOption(controllerDebug);
    parser.addOption(controllerDebugDeprecated);

    const QCommandLineOption developer(QStringLiteral("developer"),
            QCoreApplication::translate("CmdlineArgs",
                    "Enables developer-mode. Includes extra log info, stats on "
                    "performance, and a Developer tools menu."));
    parser.addOption(developer);

    const QCommandLineOption safeMode(QStringLiteral("safe-mode"),
            QCoreApplication::translate("CmdlineArgs",
                    "Enables safe-mode. Disables OpenGL waveforms, and "
                    "spinning vinyl widgets. Try this option if Mixxx is "
                    "crashing on startup."));
    QCommandLineOption safeModeDeprecated(QStringLiteral("safeMode"), safeMode.description());
    safeModeDeprecated.setFlags(QCommandLineOption::HiddenFromHelp);
    parser.addOption(safeMode);
    parser.addOption(safeModeDeprecated);

    const QCommandLineOption color(QStringLiteral("color"),
            QCoreApplication::translate("CmdlineArgs",
                    "[auto|always|never] Use colors on the console output."),
            QStringLiteral("color"),
            QStringLiteral("auto"));
    parser.addOption(color);

    const QCommandLineOption logLevel(QStringLiteral("log-level"),
            QCoreApplication::translate("CmdlineArgs",
                    "Sets the verbosity of command line logging.\n"
                    "critical - Critical/Fatal only\n"
                    "warning  - Above + Warnings\n"
                    "info     - Above + Informational messages\n"
                    "debug    - Above + Debug/Developer messages\n"
                    "trace    - Above + Profiling messages"),
            QStringLiteral("level"));
    QCommandLineOption logLevelDeprecated(QStringLiteral("logLevel"), logLevel.description());
    logLevelDeprecated.setFlags(QCommandLineOption::HiddenFromHelp);
    logLevelDeprecated.setValueName(logLevel.valueName());
    parser.addOption(logLevel);
    parser.addOption(logLevelDeprecated);

    const QCommandLineOption logFlushLevel(QStringLiteral("log-flush-level"),
            QCoreApplication::translate("CmdlineArgs",
                    "Sets the the logging level at which the log buffer is "
                    "flushed to mixxx.log. <level> is one of the values defined "
                    "at --log-level above."),
            QStringLiteral("level"));
    QCommandLineOption logFlushLevelDeprecated(
            QStringLiteral("logFlushLevel"), logLevel.description());
    logFlushLevelDeprecated.setFlags(QCommandLineOption::HiddenFromHelp);
    logFlushLevelDeprecated.setValueName(logFlushLevel.valueName());
    parser.addOption(logFlushLevel);
    parser.addOption(logFlushLevelDeprecated);

    QCommandLineOption debugAssertBreak(QStringLiteral("debug-assert-break"),
            QCoreApplication::translate("CmdlineArgs",
                    "Breaks (SIGINT) Mixxx, if a DEBUG_ASSERT evaluates to "
                    "false. Under a debugger you can continue afterwards."));
    QCommandLineOption debugAssertBreakDeprecated(
            QStringLiteral("debugAssertBreak"), debugAssertBreak.description());
    debugAssertBreakDeprecated.setFlags(QCommandLineOption::HiddenFromHelp);
    parser.addOption(debugAssertBreak);
    parser.addOption(debugAssertBreakDeprecated);

    const QCommandLineOption helpOption = parser.addHelpOption();
    const QCommandLineOption versionOption = parser.addVersionOption();

    parser.addPositionalArgument(QStringLiteral("file"),
            QCoreApplication::translate("CmdlineArgs",
                    "Load the specified music file(s) at start-up. Each file "
                    "you specify will be loaded into the next virtual deck."));

    // process all arguments
    if (!parser.parse(arguments)) {
        qWarning() << parser.errorText();
    }

    if (parser.isSet(helpOption)
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
            || parser.isSet(QStringLiteral("help-all"))
#endif
    ) {
        // we need to call process here with an initialized QCoreApplication
        // otherwise there is no way to print the help-all information
        QCoreApplication coreApp(argc, argv);
        parser.process(arguments);
        return false;
    }
    if (parser.isSet(versionOption)) {
        parser.showVersion();
        return false;
    }

    m_startInFullscreen = parser.isSet(fullScreen) || parser.isSet(fullScreenDeprecated);

    if (parser.isSet(locale)) {
        m_locale = parser.value(locale);
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

    m_midiDebug = parser.isSet(controllerDebug) || parser.isSet(controllerDebugDeprecated);
    m_developer = parser.isSet(developer);
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

    // set colors
    if (parser.value(color).compare(QLatin1String("auto"), Qt::CaseInsensitive) == 0) {
        // see https://no-color.org/
        if (QProcessEnvironment::systemEnvironment().contains(QLatin1String("NO_COLOR"))) {
            m_useColors = false;
        } else {
#ifndef __WINDOWS__
            if (isatty(fileno(stderr))) {
                m_useColors = true;
            }
#else
            if (_isatty(_fileno(stderr))) {
                m_useColors = true;
            }
#endif
        }
    } else if (parser.value(color).compare(QLatin1String("always"), Qt::CaseInsensitive) == 0) {
        m_useColors = true;
    } else if (parser.value(color).compare(QLatin1String("never"), Qt::CaseInsensitive) == 0) {
        m_useColors = false;
    } else {
        fputs("Unknown argument for for color.\n", stdout);
    }

    return true;
}
