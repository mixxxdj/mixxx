#include <stdio.h>

#include "util/cmdlineargs.h"
#include "util/version.h"

#include "sources/soundsourceproxy.h"


CmdlineArgs::CmdlineArgs()
    : m_startInFullscreen(false), // Initialize vars
      m_midiDebug(false),
      m_developer(false),
      m_safeMode(false),
      m_debugAssertBreak(false),
      m_settingsPathSet(false),
      m_logLevel(mixxx::Logging::kLogLevelDefault),
// We are not ready to switch to XDG folders under Linux, so keeping $HOME/.mixxx as preferences folder. see lp:1463273
#ifdef __LINUX__
    m_settingsPath(QDir::homePath().append("/").append(SETTINGS_PATH)) {
#else
    // TODO(XXX) Trailing slash not needed anymore as we switches from String::append
    // to QDir::filePath elsewhere in the code. This is candidate for removal.
    m_settingsPath(QDesktopServices::storageLocation(QDesktopServices::DataLocation).append("/")) {
#endif
}

bool CmdlineArgs::Parse(int &argc, char **argv) {
    bool logLevelSet = false;
    for (int i = 0; i < argc; ++i) {
        if (   argv[i] == QString("-h")
            || argv[i] == QString("--h")
            || argv[i] == QString("--help")) {
            return false; // Display Help Message
        }

        if (argv[i]==QString("-f").toLower() || argv[i]==QString("--f") || argv[i]==QString("--fullScreen"))
        {
            m_startInFullscreen = true;
        } else if (argv[i] == QString("--locale") && i+1 < argc) {
            m_locale = argv[i+1];
        } else if (argv[i] == QString("--settingsPath") && i+1 < argc) {
            m_settingsPath = QString::fromLocal8Bit(argv[i+1]);
            // TODO(XXX) Trailing slash not needed anymore as we switches from String::append
            // to QDir::filePath elsewhere in the code. This is candidate for removal.
            if (!m_settingsPath.endsWith("/")) {
                m_settingsPath.append("/");
            }
            m_settingsPathSet=true;
        } else if (argv[i] == QString("--resourcePath") && i+1 < argc) {
            m_resourcePath = QString::fromLocal8Bit(argv[i+1]);
            i++;
        } else if (argv[i] == QString("--pluginPath") && i+1 < argc) {
            m_pluginPath = QString::fromLocal8Bit(argv[i+1]);
            i++;
        } else if (argv[i] == QString("--timelinePath") && i+1 < argc) {
            m_timelinePath = QString::fromLocal8Bit(argv[i+1]);
            i++;
        } else if (argv[i] == QString("--logLevel") && i+1 < argc) {
            logLevelSet = true;
            auto level = QLatin1String(argv[i+1]);
            if (level == "debug") {
                m_logLevel = mixxx::Logging::LogLevel::Debug;
            } else if (level == "info") {
                m_logLevel = mixxx::Logging::LogLevel::Info;
            } else if (level == "warning") {
                m_logLevel = mixxx::Logging::LogLevel::Warning;
            } else if (level == "critical") {
                m_logLevel = mixxx::Logging::LogLevel::Critical;
            } else {
                fputs("\nlogLevel argument wasn't 'debug', 'info', 'warning', or 'critical'! Mixxx will only output\n\
warnings and errors to the console unless this is set properly.\n", stdout);
            }
            i++;
        } else if (QString::fromLocal8Bit(argv[i]).contains("--midiDebug", Qt::CaseInsensitive) ||
                   QString::fromLocal8Bit(argv[i]).contains("--controllerDebug", Qt::CaseInsensitive)) {
            m_midiDebug = true;
        } else if (QString::fromLocal8Bit(argv[i]).contains("--developer", Qt::CaseInsensitive)) {
            m_developer = true;
        } else if (QString::fromLocal8Bit(argv[i]).contains("--safeMode", Qt::CaseInsensitive)) {
            m_safeMode = true;
        } else if (QString::fromLocal8Bit(argv[i]).contains("--debugAssertBreak", Qt::CaseInsensitive)) {
            m_debugAssertBreak = true;
        } else {
            m_musicFiles += QString::fromLocal8Bit(argv[i]);
        }
    }

    // If --logLevel was unspecified and --developer is enabled then set
    // logLevel to debug.
    if (m_developer && !logLevelSet) {
        m_logLevel = mixxx::Logging::LogLevel::Debug;
    }

    return true;
}

void CmdlineArgs::printUsage() {
    fputs(Version::applicationName().toLocal8Bit().constData(), stdout);
    fputs(" v", stdout);
    fputs(Version::version().toLocal8Bit().constData(), stdout);
    fputs(" - Command line options", stdout);
    fputs("\n(These are case-sensitive.)\n\n\
[FILE]                  Load the specified music file(s) at start-up.\n\
                        Each must be one of the following file types:\n\
                        ", stdout);

    fputs(SoundSourceProxy::getSupportedFileNamePatterns().join(" ")
          .toLocal8Bit().constData(), stdout);
    fputs("\n\n", stdout);
    fputs("\
                        Each file you specify will be loaded into the\n\
                        next virtual deck.\n\
\n\
--resourcePath PATH     Top-level directory where Mixxx should look\n\
                        for its resource files such as MIDI mappings,\n\
                        overriding the default installation location.\n\
\n\
--pluginPath PATH       Top-level directory where Mixxx should look\n\
                        for sound source plugins in addition to default\n\
                        locations.\n\
\n\
--settingsPath PATH     Top-level directory where Mixxx should look\n\
                        for settings. Default is:\n", stdout);
    fprintf(stdout, "\
                        %s\n", getSettingsPath().toLocal8Bit().constData());
    fputs("\
\n\
--controllerDebug       Causes Mixxx to display/log all of the controller\n\
                        data it receives and script functions it loads\n\
\n\
--developer             Enables developer-mode. Includes extra log info,\n\
                        stats on performance, and a Developer tools menu.\n\
\n\
--safeMode              Enables safe-mode. Disables OpenGL waveforms,\n\
                        and spinning vinyl widgets. Try this option if\n\
                        Mixxx is crashing on startup.\n\
\n\
--locale LOCALE         Use a custom locale for loading translations\n\
                        (e.g 'fr')\n\
\n\
-f, --fullScreen        Starts Mixxx in full-screen mode\n\
\n\
--logLevel LEVEL        Sets the verbosity of command line logging\n\
                        critical - Critical/Fatal only\n\
                        warning - Above + Warnings\n\
                        info - Above + Informational messages\n\
                        debug - Above + Debug/Developer messages\n\
\n"
#ifdef MIXXX_BUILD_DEBUG
"\
--debugAssertBreak      Breaks (SIGINT) Mixxx, if a DEBUG_ASSERT\n\
                        evaluates to false. Under a debugger you can\n\
                        continue afterwards.\
\n"
#endif
"\
-h, --help              Display this help message and exit", stdout);

    fputs("\n\n(For more information, see http://mixxx.org/wiki/doku.php/command_line_options)\n",stdout);
}
