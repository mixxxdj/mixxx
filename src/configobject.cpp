/***************************************************************************
                          configobject.cpp  -  description
                             -------------------
    begin                : Thu Jun 6 2002
    copyright            : (C) 2002 by Tue & Ken Haste Andersen
    email                : haste@diku.dk
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
#include "configobject.h"

#include <QIODevice>
#include <QTextStream>
#include <QApplication>
#include <QDir>
#include <QtDebug>

#include "widget/wwidget.h"
#include "util/cmdlineargs.h"
#include "xmlparse.h"

ConfigKey::ConfigKey() {
}

ConfigKey::ConfigKey(const ConfigKey& key)
    : group(key.group),
      item(key.item) {
}

ConfigKey::ConfigKey(const QString& g, const QString& i)
    : group(g),
      item(i) {
}

// static
ConfigKey ConfigKey::parseCommaSeparated(QString key) {
    int comma = key.indexOf(",");
    ConfigKey configKey(key.left(comma), key.mid(comma + 1));
    return configKey;
}

ConfigValue::ConfigValue() {
}

ConfigValue::ConfigValue(QString stValue) {
    value = stValue;
}

ConfigValue::ConfigValue(int iValue)
{
    value = QString::number(iValue);
}

void ConfigValue::valCopy(const ConfigValue& configValue) {
    value = configValue.value;
}


ConfigValueKbd::ConfigValueKbd() {
}

ConfigValueKbd::ConfigValueKbd(QString value)
        : ConfigValue(value) {
    QString key;
    QTextStream(&value) >> key;
    m_qKey = QKeySequence(key);
}

ConfigValueKbd::ConfigValueKbd(QKeySequence key) {
    m_qKey = key;
    QTextStream(&value) << m_qKey.toString();
//          qDebug() << "value" << value;
}

void ConfigValueKbd::valCopy(const ConfigValueKbd& v) {
    m_qKey = v.m_qKey;
    QTextStream(&value) << m_qKey.toString();
}

bool operator==(const ConfigValue& s1, const ConfigValue& s2) {
    return (s1.value.toUpper() == s2.value.toUpper());
}

bool operator==(const ConfigValueKbd& s1, const ConfigValueKbd& s2) {
    return (s1.value.toUpper() == s2.value.toUpper());
}

template <class ValueType> ConfigObject<ValueType>::ConfigObject(QString file) {
    reopen(file);
}

template <class ValueType> ConfigObject<ValueType>::~ConfigObject() {
}

template <class ValueType>
void ConfigObject<ValueType>::set(const ConfigKey& k, ValueType v) {
    m_valueHash.insert(k, v);
}

template <class ValueType>
ValueType ConfigObject<ValueType>::get(const ConfigKey& k) {
    return m_valueHash.value(k, ValueType(""));
}

template <class ValueType>
bool ConfigObject<ValueType>::exists(const ConfigKey& k) {
    return m_valueHash.contains(k);
}

template <class ValueType>
QString ConfigObject<ValueType>::getValueString(const ConfigKey& k) {
    ValueType v = get(k);
    return v.value;
}

template <class ValueType>
QString ConfigObject<ValueType>::getValueString(const ConfigKey& k,
                                                const QString& default_string) {
    QString ret = getValueString(k);
    if (ret.isEmpty()) {
        return default_string;
    }
    return ret;
}

template <class ValueType> bool ConfigObject<ValueType>::parse() {
    // Open file for reading
    QFile configfile(m_filename);
    if (m_filename.length() < 1 || !configfile.open(QIODevice::ReadOnly)) {
        qDebug() << "ConfigObject: Could not read" << m_filename;
        return false;
    } else {
        //qDebug() << "ConfigObject: Parse" << m_filename;
        // Parse the file
        int group = 0;
        QString groupStr, line;
        QTextStream text(&configfile);
        text.setCodec("UTF-8");

        while (!text.atEnd()) {
            line = text.readLine().trimmed();
            if (line.length() != 0) {
                if (line.startsWith("[") && line.endsWith("]")) {
                    group++;
                    groupStr = line;
                    //qDebug() << "Group :" << groupStr;
                } else if (group > 0) {
                    QString key;
                    QTextStream(&line) >> key;
                    QString val = line.right(line.length() - key.length()); // finds the value string
                    val = val.trimmed();
                    //qDebug() << "control:" << key << "value:" << val;
                    ConfigKey k(groupStr, key);
                    ValueType m(val);
                    set(k, m);
                }
            }
        }
        configfile.close();
    }
    return true;
}

template <class ValueType> void ConfigObject<ValueType>::reopen(QString file) {
    m_filename = file;
    parse();
}

template <class ValueType> void ConfigObject<ValueType>::Save() {
    QFile file(m_filename);
    if (!QDir(QFileInfo(file).absolutePath()).exists()) {
        QDir().mkpath(QFileInfo(file).absolutePath());
    }
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Could not write file" << m_filename << ", don't worry.";
        return;
    } else {
        QTextStream stream(&file);
        stream.setCodec("UTF-8");

        QString grp = "";

        typename QHash<ConfigKey, ValueType>::iterator i;
        for (i = m_valueHash.begin(); i != m_valueHash.end(); ++i) {
            //qDebug() << "group:" << it.key().group << "item" << it.key().item << "val" << it.value()->value;
            if (i.key().group != grp) {
                grp = i.key().group;
                stream << "\n" << grp << "\n";
            }
            stream << i.key().item << " " << i.value().value << "\n";
        }
        file.close();
        if (file.error()!=QFile::NoError) { //could be better... should actually say what the error was..
            qDebug() << "Error while writing configuration file:" << file.errorString();
        }
    }
}

template <class ValueType>
QString ConfigObject<ValueType>::getResourcePath() const {
    // Try to read in the resource directory from the command line
    QString qResourcePath = CmdlineArgs::Instance().getResourcePath();

    if (qResourcePath.isEmpty()) {
        QDir mixxxDir(QCoreApplication::applicationDirPath());
        // We used to support using the mixxx.cfg's [Config],Path setting but
        // this causes issues if you try and use two different versions of Mixxx
        // on the same computer. See Bug #1392854. We start by checking if we're
        // running out of a build root ('res' dir exists or our path ends with
        // '_build') and if not then we fall back on a platform-specific method
        // of determining the resource path (see comments below).
        if (mixxxDir.cd("res")) {
            // We are running out of the repository root.
            qResourcePath = mixxxDir.absolutePath();
        } else if (mixxxDir.absolutePath().endsWith("_build") &&
                   mixxxDir.cdUp() && mixxxDir.cd("res")) {
            // We are running out of the (lin|win|osx)XX_build folder.
            qResourcePath = mixxxDir.absolutePath();
        }
#ifdef __UNIX__
        // On Linux if all of the above fail the /usr/share path is the logical
        // place to look.
        else {
            qResourcePath = UNIX_SHARE_PATH;
        }
#endif
#ifdef __WINDOWS__
        // On Windows, set the config dir relative to the application dir if all
        // of the above fail.
        else {
            qResourcePath = QCoreApplication::applicationDirPath();
        }
#endif
#ifdef __APPLE__
        else if (mixxxDir.cdUp() && mixxxDir.cd("Resources")) {
            // Release configuraton
            qResourcePath = mixxxDir.absolutePath();
        } else {
            // TODO(rryan): What should we do here?
        }
#endif
    } else {
        //qDebug() << "Setting qResourcePath from location in resourcePath commandline arg:" << qResourcePath;
    }

    if (qResourcePath.isEmpty()) {
        reportCriticalErrorAndQuit("qConfigPath is empty, this can not be so -- did our developer forget to define one of __UNIX__, __WINDOWS__, __APPLE__??");
    }

    // If the directory does not end with a "/", add one
    if (!qResourcePath.endsWith("/")) {
        qResourcePath.append("/");
    }

    qDebug() << "Loading resources from " << qResourcePath;

    return qResourcePath;
}

template <class ValueType> ConfigObject<ValueType>::ConfigObject(QDomNode node) {
    if (!node.isNull() && node.isElement()) {
        QDomNode ctrl = node.firstChild();

        while (!ctrl.isNull()) {
            if(ctrl.nodeName() == "control") {
                QString group = XmlParse::selectNodeQString(ctrl, "group");
                QString key = XmlParse::selectNodeQString(ctrl, "key");
                ConfigKey k(group, key);
                ValueType m(ctrl);
                set(k, m);
            }
            ctrl = ctrl.nextSibling();
        }
    }
}

template <class ValueType>
QString ConfigObject<ValueType>::getSettingsPath() const {
    QFileInfo configFileInfo(m_filename);
    return configFileInfo.absoluteDir().absolutePath();
}

template <class ValueType>
QHash<ConfigKey, ValueType> ConfigObject<ValueType>::toHash() const {
    return m_valueHash;
}

template class ConfigObject<ConfigValue>;
template class ConfigObject<ConfigValueKbd>;
