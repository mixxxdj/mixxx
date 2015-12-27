#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H

#include <QIODevice>
#include <QSettings>

bool readConfig(QIODevice& device, QSettings::SettingsMap& map);
bool writeConfig(QIODevice& device, const QSettings::SettingsMap& map);

class UserSettings {
  public:
    UserSettings() : m_settings(m_config_fname, m_format) {
        if (m_config_fname == "NONE") {
            // throw
        }
        m_settings.setFallbacksEnabled(false);
    }

    // prevent moving
    UserSettings(UserSettings&& rhs) = delete;
    UserSettings& operator=(UserSettings&& rhs) = delete;
    // prevent copying
    UserSettings(const UserSettings& rhs) = delete;
    UserSettings& operator=(const UserSettings& rhs) = delete;

    QSettings& getQSettings() { return m_settings; }

    static QString m_config_fname;
    static QSettings::Format m_format;

  private:
    QSettings m_settings;
};

void registerConfigPath(const QString& config_path);

#endif  // CONFIG_PARSER_H
