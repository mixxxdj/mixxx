#pragma once

#include <QDesktopServices>
#include <QDir>
#include <QList>
#include <QString>

#include "util/logging.h"

/// A structure to store the parsed command-line arguments
class CmdlineArgs final {
  public:
    /// The constructor is only public to make this class reusable in tests.
    /// All operational code in Mixxx itself must access the global singleton
    /// via `CmdlineArgs::instance()`.
    CmdlineArgs();

    static inline CmdlineArgs& Instance() {
        static CmdlineArgs cla;
        return cla;
    }

    bool Parse(int &argc, char **argv);
    void printUsage();

    const QList<QString>& getMusicFiles() const { return m_musicFiles; }
    bool getStartInFullscreen() const { return m_startInFullscreen; }
    bool getMidiDebug() const { return m_midiDebug; }
    bool getDeveloper() const { return m_developer; }
    bool getSafeMode() const { return m_safeMode; }
    bool useColors() const {
        return m_useColors;
    }
    bool getDebugAssertBreak() const { return m_debugAssertBreak; }
    bool getSettingsPathSet() const { return m_settingsPathSet; }
    mixxx::LogLevel getLogLevel() const { return m_logLevel; }
    mixxx::LogLevel getLogFlushLevel() const { return m_logFlushLevel; }
    bool getTimelineEnabled() const { return !m_timelinePath.isEmpty(); }
    const QString& getLocale() const { return m_locale; }
    const QString& getSettingsPath() const { return m_settingsPath; }
    void setSettingsPath(const QString& newSettingsPath) {
        m_settingsPath = newSettingsPath;
    }
    const QString& getResourcePath() const { return m_resourcePath; }
    const QString& getTimelinePath() const { return m_timelinePath; }

  private:
    QList<QString> m_musicFiles;    // List of files to load into players at startup
    bool m_startInFullscreen;       // Start in fullscreen mode
    bool m_midiDebug;
    bool m_developer; // Developer Mode
    bool m_safeMode;
    bool m_debugAssertBreak;
    bool m_settingsPathSet; // has --settingsPath been set on command line ?
    bool m_useColors;       // should colors be used
    mixxx::LogLevel m_logLevel; // Level of stderr logging message verbosity
    mixxx::LogLevel m_logFlushLevel; // Level of mixx.log file flushing
    QString m_locale;
    QString m_settingsPath;
    QString m_resourcePath;
    QString m_timelinePath;
};
