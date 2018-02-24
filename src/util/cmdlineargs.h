#ifndef CMDLINEARGS_H
#define CMDLINEARGS_H

#include <QList>
#include <QString>
#include <QDir>
#include <QDesktopServices>

#include "util/logging.h"

// A structure to store the parsed command-line arguments
class CmdlineArgs final {
  public:
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
    const QString& getPluginPath() const { return m_pluginPath; }
    const QString& getTimelinePath() const { return m_timelinePath; }

  private:
    CmdlineArgs();

    QList<QString> m_musicFiles;    // List of files to load into players at startup
    bool m_startInFullscreen;       // Start in fullscreen mode
    bool m_midiDebug;
    bool m_developer; // Developer Mode
    bool m_safeMode;
    bool m_debugAssertBreak;
    bool m_settingsPathSet; // has --settingsPath been set on command line ?
    mixxx::LogLevel m_logLevel; // Level of stderr logging message verbosity
    mixxx::LogLevel m_logFlushLevel; // Level of mixx.log file flushing
    QString m_locale;
    QString m_settingsPath;
    QString m_resourcePath;
    QString m_pluginPath;
    QString m_timelinePath;
};

#endif /* CMDLINEARGS_H */
