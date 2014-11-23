#ifndef CMDLINEARGS_H
#define CMDLINEARGS_H

#include <QList>
#include <QString>
#include <QDir>

// A structure to store the parsed command-line arguments
class CmdlineArgs {
  public:
    static inline CmdlineArgs& Instance() {
        static CmdlineArgs cla;
        return cla;
    }
    bool Parse(int &argc, char **argv) {
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
                if (!m_settingsPath.endsWith("/")) {
                    m_settingsPath.append("/");
                }
            } else if (argv[i] == QString("--resourcePath") && i+1 < argc) {
                m_resourcePath = QString::fromLocal8Bit(argv[i+1]);
                i++;
            } else if (argv[i] == QString("--pluginPath") && i+1 < argc) {
                m_pluginPath = QString::fromLocal8Bit(argv[i+1]);
                i++;
            } else if (argv[i] == QString("--timelinePath") && i+1 < argc) {
                m_timelinePath = QString::fromLocal8Bit(argv[i+1]);
                i++;
            } else if (QString::fromLocal8Bit(argv[i]).contains("--midiDebug", Qt::CaseInsensitive) ||
                       QString::fromLocal8Bit(argv[i]).contains("--controllerDebug", Qt::CaseInsensitive)) {
                m_midiDebug = true;
            } else if (QString::fromLocal8Bit(argv[i]).contains("--developer", Qt::CaseInsensitive)) {
                m_developer = true;

            } else if (QString::fromLocal8Bit(argv[i]).contains("--safeMode", Qt::CaseInsensitive)) {
                m_safeMode = true;
            } else {
                m_musicFiles += QString::fromLocal8Bit(argv[i]);
            }
        }
        return true;
    }
    const QList<QString>& getMusicFiles() const { return m_musicFiles; }
    bool getStartInFullscreen() const { return m_startInFullscreen; }
    bool getMidiDebug() const { return m_midiDebug; }
    bool getDeveloper() const { return m_developer; }
    bool getSafeMode() const { return m_safeMode; }
    bool getTimelineEnabled() const { return !m_timelinePath.isEmpty(); }
    const QString& getLocale() const { return m_locale; }
    const QString& getSettingsPath() const { return m_settingsPath; }
    const QString& getResourcePath() const { return m_resourcePath; }
    const QString& getPluginPath() const { return m_pluginPath; }
    const QString& getTimelinePath() const { return m_timelinePath; }

  private:
    CmdlineArgs() :
        m_startInFullscreen(false), //Initialize vars
        m_midiDebug(false),
        m_developer(false),
        m_safeMode(false),
        m_settingsPath(QDir::homePath().append("/").append(SETTINGS_PATH)) {
    }
    ~CmdlineArgs() { };

    QList<QString> m_musicFiles;    /* List of files to load into players at startup */
    bool m_startInFullscreen;       /* Start in fullscreen mode */
    bool m_midiDebug;
    bool m_developer; // Developer Mode
    bool m_safeMode;
    QString m_locale;
    QString m_settingsPath;
    QString m_resourcePath;
    QString m_pluginPath;
    QString m_timelinePath;
};

#endif /* CMDLINEARGS_H */
