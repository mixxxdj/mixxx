#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H

#include <QIODevice>
#include <QSettings>

bool readConfig(QIODevice& device, QSettings::SettingsMap& map);
bool writeConfig(QIODevice& device, const QSettings::SettingsMap& map);

#endif  // CONFIG_PARSER_H
