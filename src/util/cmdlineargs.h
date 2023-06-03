#pragma once

#include <QCoreApplication>
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

    //! The original parser that provides the parsed values to Mixxx
    //! This can be called before anything else is initialized
    bool parse(int argc, char** argv);

    //! The optional second run, that provides translated user feedback
    //! This requires an initialized QCoreApplication
    void parseForUserFeedback();

    const QList<QString>& getMusicFiles() const { return m_musicFiles; }
    bool getStartInFullscreen() const { return m_startInFullscreen; }
    bool getControllerDebug() const {
        return m_controllerDebug;
    }
    bool getDeveloper() const { return m_developer; }
    bool getSafeMode() const { return m_safeMode; }
    bool useColors() const {
        return m_useColors;
    }
    bool getUseVuMeterGL() const {
        return m_useVuMeterGL;
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

    void setScaleFactor(double scaleFactor) {
        m_scaleFactor = scaleFactor;
    }
    double getScaleFactor() const {
        return m_scaleFactor;
    }
    const QString& getPlaylistFilePath() const { return m_playlistFilePath; }

  private:
    enum class ParseMode {
        Initial,
        ForUserFeedback
    };

    bool parse(const QStringList& arguments, ParseMode mode);

    QList<QString> m_musicFiles;    // List of files to load into players at startup
    bool m_startInFullscreen;       // Start in fullscreen mode
    bool m_controllerDebug;
    bool m_developer; // Developer Mode
    bool m_safeMode;
    bool m_useVuMeterGL;
    bool m_debugAssertBreak;
    bool m_settingsPathSet; // has --settingsPath been set on command line ?
    double m_scaleFactor;
    bool m_useColors;       // should colors be used
    bool m_parseForUserFeedbackRequired;
    mixxx::LogLevel m_logLevel; // Level of stderr logging message verbosity
    mixxx::LogLevel m_logFlushLevel; // Level of mixx.log file flushing
    QString m_locale;
    QString m_settingsPath;
    QString m_resourcePath;
    QString m_timelinePath;
    QString m_playlistFilePath; // New member variable to store the playlist path
};
