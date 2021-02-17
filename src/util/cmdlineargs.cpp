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

#include "sources/soundsourceproxy.h"
#include "util/version.h"

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
#ifdef __LINUX__
          m_settingsPath(QDir::homePath().append("/").append(SETTINGS_PATH)) {
#else
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
    if (logLevel == QLatin1String("trace")) {
        *pLogLevel = mixxx::LogLevel::Trace;
    } else if (logLevel == QLatin1String("debug")) {
        *pLogLevel = mixxx::LogLevel::Debug;
    } else if (logLevel == QLatin1String("info")) {
        *pLogLevel = mixxx::LogLevel::Info;
    } else if (logLevel == QLatin1String("warning")) {
        *pLogLevel = mixxx::LogLevel::Warning;
    } else if (logLevel == QLatin1String("critical")) {
        *pLogLevel = mixxx::LogLevel::Critical;
    } else {
        return false;
    }
    return true;
}
} // namespace

bool CmdlineArgs::Parse(const QStringList& arguments) {
    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::translate("main",
            "Mixxx is an open source DJ software. For more information, see "
            "https://manual.mixxx.org/2.3/chapters/appendix.html#command-line-options)"));
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);

    // add options
    const QCommandLineOption fullscreen(QStringList() << "f"
                                                      << "fullScreen",
            QCoreApplication::translate("main", "Starts Mixxx in full-screen mode"));
    parser.addOption(fullscreen);

    const QCommandLineOption locale(QStringLiteral("locale"),
            QCoreApplication::translate("main",
                    "Use a custom locale for loading translations. (e.g "
                    "'fr')"));
    parser.addOption(locale);

    // An option with a value
    const QCommandLineOption settingsPath(QStringLiteral("settingsPath"),
            QCoreApplication::translate("main",
                    "Top-level directory where Mixxx should look for settings. "
                    "Default is:") +
                    getSettingsPath().toLocal8Bit().constData(),
            QStringLiteral("settingsPath"));
    parser.addOption(settingsPath);

    const QCommandLineOption pluginPath(QStringLiteral("pluginPath"),
            QCoreApplication::translate("main",
                    "Top-level directory where Mixxx should look for sound "
                    "source plugins in addition to default locations."),
            QStringLiteral("pluginPath"));
    parser.addOption(pluginPath);

    QCommandLineOption resourcePath(QStringLiteral("resourcePath"),
            QCoreApplication::translate("main",
                    "Top-level directory where Mixxx should look for its "
                    "resource files such as MIDI mappings, overriding the "
                    "default installation location."),
            QStringLiteral("resourcePath"));
    parser.addOption(resourcePath);

    const QCommandLineOption timelinePath(QStringLiteral("timelinePath"),
            QCoreApplication::translate("main", "Path the timeline is written to"));
    parser.addOption(timelinePath);

    const QCommandLineOption controllerDebug(QStringList() << "controllerDebug"
                                                           << "midiDebug",
            QCoreApplication::translate("main",
                    "Causes Mixxx to display/log all of the controller data it "
                    "receives and script functions it loads"));
    parser.addOption(controllerDebug);

    const QCommandLineOption developer(QStringLiteral("developer"),
            QCoreApplication::translate("main",
                    "Enables developer-mode. Includes extra log info, stats on "
                    "performance, and a Developer tools menu."));
    parser.addOption(developer);

    const QCommandLineOption safeMode(QStringLiteral("safeMode"),
            QCoreApplication::translate("main",
                    "Enables safe-mode. Disables OpenGL waveforms, and "
                    "spinning vinyl widgets. Try this option if Mixxx is "
                    "crashing on startup."));
    parser.addOption(safeMode);

    const QCommandLineOption color(QStringLiteral("color"),
            QCoreApplication::translate("main",
                    "Enables safe-mode. Disables OpenGL waveforms, and "
                    "spinning vinyl widgets. Try this option if Mixxx is "
                    "crashing on startup."),
            QStringLiteral("color"),
            QStringLiteral("auto"));
    parser.addOption(color);

    const QCommandLineOption logLevel(QStringLiteral("logLevel"),
            QCoreApplication::translate("main",
                    "Sets the verbosity of command line logging.\n"
                    "critical - Critical/Fatal only\n"
                    "warning  - Above + Warnings"
                    "info     - Above + Informational messages\n"
                    "debug    - Above + Debug/Developer messages\n"
                    "trace    - Above + Profiling messages"),
            QStringLiteral("logLevel"));
    parser.addOption(logLevel);

    const QCommandLineOption logFlushLevel(QStringLiteral("logFlushLevel"),
            QCoreApplication::translate("main",
                    "Sets the the logging level at which the log buffer is "
                    "flushed to mixxx.log. LEVEL is one of the values defined "
                    "at --logLevel above."),
            QStringLiteral("logFlushLevel"));
    parser.addOption(logFlushLevel);

#ifdef MIXXX_BUILD_DEBUG
    QCommandLineOption debugAssertBreak(QStringLiteral("debugAssertBreak"),
            QCoreApplication::translate("main",
                    "Breaks (SIGINT) Mixxx, if a DEBUG_ASSERT evaluates to "
                    "false. Under a debugger you can continue afterwards."));
    parser.addOption(debugAssertBreak);
#endif

    const QCommandLineOption helpOption = parser.addHelpOption();
    const QCommandLineOption versionOption = parser.addVersionOption();

    parser.addPositionalArgument(QStringLiteral("file"),
            QCoreApplication::translate("main",
                    "Load the specified music file(s) at start-up. Each file "
                    "you specify will be loaded into the next virtual deck."));

    // process all arguments
    if (!parser.parse(arguments)) {
        qWarning() << parser.errorText();
    }

    if (parser.isSet(helpOption) || parser.isSet(QStringLiteral("help-all"))) {
        // we need to call process here otherwise there is no way to print the
        // help-all informations
        parser.process(arguments);
        return false;
    }
    if (parser.isSet(versionOption)) {
        parser.showVersion();
        return false;
    }

    m_startInFullscreen = parser.isSet(fullscreen);

    if (parser.isSet(locale)) {
        m_locale = parser.value(locale);
    }

    if (parser.isSet(settingsPath)) {
        m_settingsPath = parser.value(settingsPath);
        if (!m_settingsPath.endsWith("/")) {
            m_settingsPath.append("/");
        }
        m_settingsPathSet = true;
    }

    if (parser.isSet(resourcePath)) {
        m_resourcePath = parser.value(resourcePath);
    }

    if (parser.isSet(pluginPath)) {
        m_pluginPath = parser.value(pluginPath);
    }
    m_midiDebug = parser.isSet(controllerDebug);
    m_developer = parser.isSet(developer);
    m_safeMode = parser.isSet(safeMode);

#ifdef MIXXX_BUILD_DEBUG
    m_debugAssertBreak = parser.isSet(debugAssertBreak);
#endif

    m_musicFiles = parser.positionalArguments();

    if (parser.isSet(logLevel)) {
        if (!parseLogLevel(parser.value(logLevel), &m_logLevel)) {
            fputs("\nlogLevel argument wasn't 'trace', 'debug', 'info', 'warning', or 'critical'! Mixxx will only output\n\
warnings and errors to the console unless this is set properly.\n",
                    stdout);
        }
    } else {
        if (m_developer) {
            m_logLevel = mixxx::LogLevel::Debug;
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
        qWarning() << "Unknown setting for color, ignoring";
    }

    return true;
}
