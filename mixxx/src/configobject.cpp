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
#include <qapplication.h>
#include "configobject.h"
#include <qdir.h>
#include <QtDebug>
#include "widget/wwidget.h"

#ifdef __WINDOWS__
#include <windows.h>
#endif

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

#include <qiodevice.h>
#include <QTextStream>
#include <math.h>

ConfigKey::ConfigKey()
{
}

ConfigKey::ConfigKey(QString g, QString i)
{
    group = g;
    item = i;
}

// static
ConfigKey ConfigKey::parseCommaSeparated(QString key) {
    ConfigKey configKey;
    int comma = key.indexOf(",");
    configKey.group = key.left(comma);
    configKey.item = key.mid(comma+1);
    return configKey;
}

ConfigValue::ConfigValue()
{
}

ConfigValue::ConfigValue(QString _value)
{
    value = _value;
}

ConfigValue::ConfigValue(int _value)
{
    value = QString::number(_value);
}

void ConfigValue::valCopy(const ConfigValue _value)
{
    value = _value.value;
}


ConfigValueKbd::ConfigValueKbd()
{
}

ConfigValueKbd::ConfigValueKbd(QString _value) : ConfigValue(_value)
{
    QString key;

    QTextStream(&_value) >> key;
    m_qKey = QKeySequence(key);
}

ConfigValueKbd::ConfigValueKbd(QKeySequence key)
{
    m_qKey = key;
    QTextStream(&value) << m_qKey.toString();
//          qDebug() << "value" << value;
}

void ConfigValueKbd::valCopy(const ConfigValueKbd v)
{
    m_qKey = v.m_qKey;
    QTextStream(&value) << m_qKey.toString();
}

bool operator==(const ConfigValue & s1, const ConfigValue & s2)
{
    return (s1.value.toUpper() == s2.value.toUpper());
}

bool operator==(const ConfigValueKbd & s1, const ConfigValueKbd & s2)
{
    return (s1.value.toUpper() == s2.value.toUpper());
}

template <class ValueType> ConfigObject<ValueType>::ConfigObject(QString file)
{
    reopen(file);
}

template <class ValueType> ConfigObject<ValueType>::~ConfigObject()
{
    while (list.size() > 0) {
        ConfigOption<ValueType>* pConfigOption = list.takeLast();
        delete pConfigOption;
    }
}

template <class ValueType>
ConfigOption<ValueType> *ConfigObject<ValueType>::set(ConfigKey k, ValueType v)
{
    // Search for key in list, and set value if found
    QListIterator<ConfigOption<ValueType>* > iterator(list);
    ConfigOption<ValueType>* it;
    while (iterator.hasNext())
    {
        it = iterator.next();
//         if (QString::compare(it->val->value, v.value, Qt::CaseInsensitive) == 0)
        if (it->key->group == k.group && it->key->item == k.item)
        {
            //qDebug() << "set found." << group << "," << item;
            //cout << "1: " << v.value << "\n";
            //qDebug() << "configobject " << it->val;
            it->val->valCopy(v); // Should be done smarter using object copying
            //qDebug() << "configobject " << it->val;
            //cout << "2: " << it->val->value << "\n";
            return it;
        }
    }

    // If key is not found, insert it into the list of config objects
    ConfigKey * key = new ConfigKey(k.group, k.item);
    it = new ConfigOption<ValueType>(key, new ValueType(v));
    //qDebug() << "new configobject " << it->val;
    list.append(it);
    return it;
}

template <class ValueType>
ConfigOption<ValueType> *ConfigObject<ValueType>::get(ConfigKey k)
{
    QListIterator<ConfigOption<ValueType>* > iterator(list);
    ConfigOption<ValueType>* it;
    while (iterator.hasNext())
    {
        it = iterator.next();
        //qDebug() << it->key->group << k->group << it->key->item << k->item;
        if (it->key->group == k.group && it->key->item == k.item)
        {
            //cout << it->key->group << ":" << it->key->item << ", val: " << it->val->value << "\n";
            return it;
        }
    }
    // If key is not found, insert into list with null values
    ConfigKey * key = new ConfigKey(k.group, k.item);
    it = new ConfigOption<ValueType>(key, new ValueType(""));
    list.append(it);
    return it;
}

template <class ValueType>
ConfigKey *ConfigObject<ValueType>::get(ValueType v)
{
    QListIterator<ConfigOption<ValueType>* > iterator(list);
    ConfigOption<ValueType>* it;
    while (iterator.hasNext())
    {
        it = iterator.next();
        if (QString::compare(it->val->value, v.value, Qt::CaseInsensitive) == 0){
            //qDebug() << "ConfigObject #534: QString::compare match for " << it->key->group << it->key->item;
            return it->key;
        }
        if (((ValueType)*it->val) == ((ValueType)v))
        {
            //qDebug() << "ConfigObject: match" << it->val->value.toUpper() << "with" << v.value.toUpper();
            return it->key;
        }

        if (it == list.last()) {
            //qDebug() << "ConfigObject: last match attempted" << it->val->value.toUpper() << "with" << v.value.toUpper();
        }
    }
    //qDebug() << "No match for ConfigObject:" << v.value;
    return 0;
}

template <class ValueType>
QString ConfigObject<ValueType>::getValueString(ConfigKey k)
{
    return get(k)->val->value;
}

template <class ValueType>
QString ConfigObject<ValueType>::getValueString(ConfigKey k, const QString& default_string)
{
    QString ret = get(k)->val->value;
    if (ret.isEmpty()) {
        return default_string;
    }
    return ret;
}

template <class ValueType> bool ConfigObject<ValueType>::Parse()
{
    // Open file for reading
    QFile configfile(filename);
    if (filename.length()<1 || !configfile.open(QIODevice::ReadOnly))
    {
        qDebug() << "Could not read" << filename;
        return false;
    }
    else
    {
        // Parse the file
        int group = 0;
        QString groupStr, line;
        QTextStream text(&configfile);
        text.setCodec("UTF-8");

        while (!text.atEnd())
        {
            line = text.readLine().trimmed();

            if (line.length() != 0)
			{
                if (line.startsWith("[") && line.endsWith("]"))
                {
                    group++;
                    groupStr = line;
                    //qDebug() << "Group :" << groupStr;
                }
                else if (group>0)
                {
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


template <class ValueType> void ConfigObject<ValueType>::clear()
{
    //Delete the pointers, because that's what we did before we
    //purged Mixxx of Qt3 code. -- Albert, June 18th 2010 (at 30,000 ft)
    for (int i = 0; i < list.count(); i++)
        delete list[i];

    // This shouldn't be done, since objects might have references to
    // members of list. Instead all member values should be set to some
    // null value.
    list.clear();

}

template <class ValueType> void ConfigObject<ValueType>::reopen(QString file)
{
    // First try to open the config file placed in the users .mixxx directory.
    // If that fails, try the system wide CONFIG_PATH.

    filename = file;
    Parse();
}

template <class ValueType> void ConfigObject<ValueType>::Save()
{
    QFile file(filename);
    if (!QDir(QFileInfo(file).absolutePath()).exists()) {
        QDir().mkpath(QFileInfo(file).absolutePath());
    }
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "Could not write file" << filename << ", don't worry.";
        return;
    }
    else
    {
        QTextStream stream(&file);
        stream.setCodec("UTF-8");

        QString grp = "";

        QListIterator<ConfigOption<ValueType>* > iterator(list);
        ConfigOption<ValueType>* it;
        while (iterator.hasNext())
        {
            it = iterator.next();
//            qDebug() << "group:" << it->key->group << "item" << it->key->item << "val" << it->val->value;
            if (it->key->group != grp)
            {
                grp = it->key->group;
                stream << "\n" << it->key->group << "\n";
            }
            stream << it->key->item << " " << it->val->value << "\n";
        }
        file.close();
        if (file.error()!=QFile::NoError) //could be better... should actually say what the error was..
	  qDebug() << "Error while writing configuration file:" << file.errorString();
    }
}

template <class ValueType>
QString ConfigObject<ValueType>::getConfigPath()
{
    //
    // Find the config path, path where midi configuration files, skins etc. are stored.
    // On Unix the search order is whats listed in mixxx.cfg, then UNIX_SHARE_PATH
    // On Windows it is always (and only) app dir.
    // On OS X it is the current directory and then the Resources/ dir in the app bundle
    //
    QString qConfigPath; // TODO: this only changes once (on first load) during a run should make this a singleton.

    // Try to read in the resource directory from the command line
    QStringList commandLineArgs = QApplication::arguments();
    int resourcePath = commandLineArgs.indexOf("--resourcePath");
    if (resourcePath!=-1 && resourcePath + 1 < commandLineArgs.size()) {
        qDebug() << "Setting qConfigPath from location in resourcePath commandline arg:" << commandLineArgs.at(resourcePath+1);
        qConfigPath = commandLineArgs.at(resourcePath+1);
    }
    if (qConfigPath.isNull() || qConfigPath.isEmpty()) {
#ifdef __UNIX__
    // On Linux, check if the path is stored in the configuration database.
    if (getValueString(ConfigKey("[Config]","Path")).length()>0 && QDir(getValueString(ConfigKey("[Config]","Path"))).exists())
        qConfigPath = getValueString(ConfigKey("[Config]","Path"));
    else
    {
        // Set the path according to the compile time define, UNIX_SHARE_PATH
        qConfigPath = UNIX_SHARE_PATH;
    }
#endif
#ifdef __WINDOWS__
    // On Windows, set the config dir relative to the application dir
    qConfigPath = QCoreApplication::applicationDirPath();
#endif
#ifdef __APPLE__
    /*
    // Set the path relative to the bundle directory
    CFURLRef pluginRef = CFBundleCopyBundleURL(CFBundleGetMainBundle());
    CFStringRef macPath = CFURLCopyFileSystemPath(pluginRef, kCFURLPOSIXPathStyle);
    char utf8path[256];
    //Attempt to decode obtain the macPath string as UTF-8
    if (CFStringGetCString(macPath, utf8path, sizeof(utf8path), kCFStringEncodingUTF8))
    {
        qConfigPath.fromUtf8(utf8path);
    }
    else {
        //Fallback on the "system encoding"... (this is just our old code, which probably doesn't make any sense
         //since it plays roullette with the type of text encoding)
        qConfigPath = CFStringGetCStringPtr(macPath, CFStringGetSystemEncoding());
    }
    qConfigPath.append("/Contents/Resources/"); //XXX this should really use QDir, this entire function should
    */
    QString mixxxPath = QCoreApplication::applicationDirPath();
    if (mixxxPath.endsWith("osx_build"))   //Development configuration
        qConfigPath = mixxxPath + "/../res";
    else //Release configuraton
	    qConfigPath = mixxxPath + "/../Resources";
#endif
    }
	if (qConfigPath.length() == 0) qCritical() << "qConfigPath is empty, this can not be so -- did our developer forget to define one of __UNIX__, __WINDOWS__, __APPLE__??";
    // If the directory does not end with a "/", add one
    if (!qConfigPath.endsWith("/"))
        qConfigPath.append("/");

    return qConfigPath;
}


template <class ValueType> ConfigObject<ValueType>::ConfigObject(QDomNode node) {

    if (!node.isNull() && node.isElement()) {
        QDomNode ctrl = node.firstChild();

        while (!ctrl.isNull()) {
            if(ctrl.nodeName() == "control") {
                QString group = WWidget::selectNodeQString(ctrl, "group");
                QString key = WWidget::selectNodeQString(ctrl, "key");
                ConfigKey k(group, key);
                ValueType m(ctrl);
                set(k, m);
            }
            ctrl = ctrl.nextSibling();
        }
    }
}


template class ConfigObject<ConfigValue>;
template class ConfigObject<ConfigValueKbd>;

