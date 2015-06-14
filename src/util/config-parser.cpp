#include <QApplication>
#include <QDir>
#include <QIODevice>
#include <QRegExp>
#include <QTextStream>
#include <QtDebug>

#include "util/config-parser.h"

bool readConfig(QIODevice &device, QSettings::SettingsMap &map) {
    bool found_group = false;
    QString groupStr, line;
    QTextStream configfile(&device);
    configfile.setCodec("UTF-8");

    while (!configfile.atEnd()) {
        line = configfile.readLine().trimmed();
        if (line.length() != 0) {
            if (line.startsWith("[") && line.endsWith("]")) {
                found_group = true;
                groupStr = line;
                groupStr.remove(QRegExp("[\\[\\]]"));
            } else if (found_group) {
                QString key, val;
                QTextStream(&line) >> key >> val;
                map.insert(groupStr + "/" + key, QVariant(val));
            }
        }
    }

    return true;
}

bool writeConfig(QIODevice &device, const QSettings::SettingsMap &map) {
    QTextStream configfile(&device);
    configfile.setCodec("UTF-8");

    QString grp = "";
    // can't simply use c++11 for-each loop see
    // http://stackoverflow.com/a/8529237/2207958
    for (auto el = map.begin(), end = map.end(); el != end; ++el) {
        QStringList group_key = el.key().split("/");
        QString group = "[" + group_key.at(0) + "]";
        QString key = group_key.at(1);
        QString val = el.value().toString();
        if (group != grp) {
            grp = group;
            configfile << "\n" << group << "\n";
        }
        configfile << key << " " << val << "\n";
    }

    return true;
}
