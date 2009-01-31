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
#include <qdir.h>
#include <QtDebug>
#include "widget/wwidget.h"

#ifdef __C_METRICS__
#include "cmetrics.h"
#endif

#ifdef __WIN32__
#include <windows.h>
#endif

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

#include <qiodevice.h>
#include <QTextIStream>
#include <math.h>

ConfigKey::ConfigKey()
{
}

ConfigKey::ConfigKey(QString g, QString i)
{
    group = g;
    item = i;
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

ConfigValueMidi::ConfigValueMidi()
{
}

ConfigValueMidi::ConfigValueMidi(QDomNode node) {
    QString key = WWidget::selectNodeQString(node, "key");
    QString type = WWidget::selectNodeQString(node, "miditype");
    midino = WWidget::selectNodeInt(node, "midino");
    midichannel = WWidget::selectNodeInt(node, "midichan");

    // BJW: Jog/scratch sensitivity adjustment
    sensitivity = WWidget::selectNodeInt(node, "sensitivity");
    if (sensitivity) {
        qDebug() << "Setting" << key << "sensitivity to" << sensitivity;
    } else if (key == "wheel" || key == "scratch" || key == "jog") {
        sensitivity = 50;
        qDebug() << "Defaulting" << key << "sensitivity to" << sensitivity;
    }

    // BJW: MIDI Value Translations
    QDomNode translations = node.namedItem("translations");
    if (!translations.isNull()) {
        QDomNode translation = translations.firstChild();
        while (!translation.isNull()) {
            QDomNamedNodeMap attrs = translation.attributes();
            if (translation.nodeName() == "single") {
                if (attrs.namedItem("value").isNull()) {
                    qWarning("MIDI Map: Missing 'value' attribute in <translations><single>");
                } else {
                    int from = attrs.namedItem("value").toAttr().value().toInt();
                    int newval = translation.toElement().text().toInt();
                    if (from < -128 || from > 127 || newval < -128 || newval > 127) {
                        qWarning("MIDI Map: Illegal values in <translations><single>");
                    } else {
                        translateMidiValues[(char) from] = (char) newval;
                        qDebug() << "MIDI Map: Value Translation: Single value " << from << " -> " << newval;
                    }
                }
            } else if (translation.nodeName() == "range") {
                if (attrs.namedItem("lower").isNull() || attrs.namedItem("upper").isNull()) {
                    qWarning("MIDI Map: Missing 'lower' and/or 'upper' attribute in <translations><range>");
                } else {
                    int lower = attrs.namedItem("lower").toAttr().value().toInt();
                    int upper = attrs.namedItem("upper").toAttr().value().toInt();
                    int newval = translation.toElement().text().toInt();
                    if (lower < -128 || lower > 127 || upper < -128 || upper > 127 || upper < lower || newval < -128 || newval > 127) {
                        qWarning("MIDI Map: Illegal values in <translations><range>");
                    } else {
                        for (char c = lower; c <= upper; c++) {
                            translateMidiValues[c] = (char) newval;
                        }
                        qDebug() << "MIDI Map: Value Translation: Range of values " << lower << "-" << upper << " -> " << newval;
                    }
                }
            } else {
                qWarning() << "MIDI Map: Unknown translation element:" << translation.nodeName();
            }
            translation = translation.nextSibling();
        }
    }

    if (midichannel == 0) { midichannel = 1; }

    QTextOStream(&value) << midino << " ch " << midichannel;

    type = type.lower();

    // BJW: Try to spot missing XML blocks. I don't know how to make these warnings
    // more useful by adding the line number of the input file.
    if ((key.isEmpty() || type.isEmpty()) && node.nodeName() != "#comment")
        qWarning() << "Missing <key> or <type> in MIDI map node:" << node.nodeName();
    if (!midino && type != "pitch")
        qWarning() << "No <midino> defined in MIDI map node:" << node.nodeName();

    if (type == "key" || type == "note") {
        miditype = MIDI_KEY;
        value.prepend("Key ");
    } else if (type == "ctrl") {
        miditype = MIDI_CTRL;
        value.prepend("Ctrl ");
    } else if (type == "pitch") {
        miditype = MIDI_PITCH;
        value.prepend("Pitch ");
    } else {
        miditype = MIDI_EMPTY;
    }

    QDomNode opts = node.namedItem("options");
    if (!opts.isNull()) {
        QDomNode opt = opts.firstChild();
        if (!opt.nextSibling().isNull()) {
            qCritical("Multiple option elements in midi mapping not supported yet");
        }

        QString optname = opt.nodeName().lower();
        if (optname == "invert")
            midioption = MIDI_OPT_INVERT;
        else if (optname == "rot64inv")
            midioption = MIDI_OPT_ROT64_INV;
        else if (optname == "rot64fast")
            midioption = MIDI_OPT_ROT64_FAST;
        else if (optname == "rot64")
            midioption = MIDI_OPT_ROT64;
        else if (optname == "diff")
            midioption = MIDI_OPT_DIFF;
        else if (optname == "button")
            midioption = MIDI_OPT_BUTTON;
        else if (optname == "switch")
            midioption = MIDI_OPT_SWITCH;
        else if (optname == "hercjog")
            midioption = MIDI_OPT_HERC_JOG;
        else if (optname == "spread64")
            midioption = MIDI_OPT_SPREAD64;
        else if (optname == "selectknob")
        	midioption = MIDI_OPT_SELECTKNOB;
        else if (optname == "script-binding")
        	midioption = MIDI_OPT_SCRIPT;
        else {
            qWarning() << "Unknown option:" << optname;
            midioption = MIDI_OPT_NORMAL;
        }

        qDebug() << "Found option: " << optname << "(" << midioption << ")";
    } else {
        midioption = MIDI_OPT_NORMAL;
    }

    // qDebug() << "MIDI Config:" << key << "=" << value << "option" << midioption;

}

ConfigValueMidi::ConfigValueMidi(QString _value)
{
    qDebug() << "ConfigValueMidi(QString)";
    QString channelMark;
    QString type;
    QString option;

    QTextIStream(&_value) >> type >> midino >> channelMark >> midichannel >> option;
    if (type.contains("Key",false))
        miditype = MIDI_KEY;
    else if (type.contains("Ctrl",false))
        miditype = MIDI_CTRL;
    else if (type.contains("Pitch",false))
        miditype = MIDI_PITCH;
    else
        miditype = MIDI_EMPTY;

    if (!channelMark.endsWith("h")) {
        // No channel specified; default to channel 1 and parse this field as an option instead
        option = channelMark;
        midichannel = 1;
    }


    // If empty string, default midino should be below 0.
    if (_value.length()==0)
        midino = -1;
    qDebug() << "miditype:" << type << "midino" << midino << "midichannel" << midichannel;

    if (option.contains("Invert", false))
        midioption = MIDI_OPT_INVERT;
    else if (option.contains("Rot64Inv", false))
        midioption = MIDI_OPT_ROT64_INV;
    else if (option.contains("Rot64Fast", false))
        midioption = MIDI_OPT_ROT64_FAST;
    else if (option.contains("Rot64", false))
        midioption = MIDI_OPT_ROT64;
    else if (option.contains("Diff", false))
        midioption = MIDI_OPT_DIFF;
    else if (option.contains("Button", false))
        midioption = MIDI_OPT_BUTTON;
    else if (option.contains("Switch", false))
        midioption = MIDI_OPT_SWITCH;
    else if (option.contains("HercJog", false))
        midioption = MIDI_OPT_HERC_JOG;
    else if (option.contains("Spread64", false))
        midioption = MIDI_OPT_SPREAD64;
    else if (option.contains("SelectKnob", false))
        midioption = MIDI_OPT_SELECTKNOB;
    else if (option.contains("Script-Binding", false))
        midioption = MIDI_OPT_SCRIPT;
    else
        midioption = MIDI_OPT_NORMAL;
    // Store string with corrected config value
    //value="";
    //QTextOStream(&value) << type << " " << midino << " ch " << midichannel;
}

ConfigValueMidi::ConfigValueMidi(MidiType _miditype, int _midino, int _midichannel)
{
    //qDebug() << "--2";
    miditype = _miditype;
    midino = _midino;
    midichannel = _midichannel;
    QTextOStream(&value) << midino << " ch " << midichannel;
    if (miditype==MIDI_KEY)
        value.prepend("Key ");
    else if (miditype==MIDI_CTRL)
        value.prepend("Ctrl ");
    else if (miditype==MIDI_PITCH)
        value.prepend("Pitch ");
    midioption = MIDI_OPT_NORMAL;

    //QTextIStream(&value) << midino << midimask << "ch" << midichannel;
}

void ConfigValueMidi::valCopy(const ConfigValueMidi v)
{
    //qDebug() << "--1, midino: " << midino << ", midimask: " << midimask << ", midichannel: " << midichannel;
    miditype = v.miditype;
    midino = v.midino;
    midichannel = v.midichannel;
    midioption = v.midioption;
    value = "";
    QTextOStream(&value) << midino << " ch " << midichannel;
    if (miditype==MIDI_KEY)
        value.prepend("Key ");
    else if (miditype==MIDI_CTRL)
        value.prepend("Ctrl ");
    else if (miditype==MIDI_PITCH)
        value.prepend("Pitch ");

    if (midioption == MIDI_OPT_INVERT)
        value.append(" Invert");
    else if (midioption == MIDI_OPT_ROT64)
        value.append(" Rot64");
    else if (midioption == MIDI_OPT_ROT64_INV)
        value.append(" Rot64Inv");
    else if (midioption == MIDI_OPT_ROT64_FAST)
        value.append(" Rot64Fast");
    else if (midioption == MIDI_OPT_DIFF)
        value.append(" Diff");
    else if (midioption == MIDI_OPT_SPREAD64)
        value.append(" Spread64");
    else if (midioption == MIDI_OPT_SELECTKNOB)
        value.append(" SelectKnob");

    qDebug() << "Config value:" << value;
    //qDebug() << "--1, midino: " << midino << ", midimask: " << midimask << ", midichannel: " << midichannel;
}


// BJW: Apply value translations defined in MIDI map file. Done separately to ComputeValue() as these need
// to be done prior to any midioption-related activity which might change the original MIDI value (and outputs
// a double rather than a char)
char ConfigValueMidi::translateValue(char value)
{
    if (!translateMidiValues.isEmpty()) {
        MidiValueMap::Iterator it;
        if ((it = translateMidiValues.find(value)) != translateMidiValues.end()) {
            // qDebug() << "MIDI value " << value << " translated to " << it.data();
            return it.data();
        }
    }
    return value;
}


// BJW: Note: _prevmidivalue is not the previous MIDI value. It's the
// current controller value, scaled to 0-127 but only in the case of pots.
// (See Control*::GetMidiValue())
double ConfigValueMidi::ComputeValue(MidiType /* _miditype */, double _prevmidivalue, double _newmidivalue)
{
    double tempval = 0.;
    double diff = 0.;

    // qDebug() << "ComputeValue: option " << midioption << ", MIDI value " << _newmidivalue << ", current control value " << _prevmidivalue;
    if (midioption == MIDI_OPT_NORMAL) {
        return _newmidivalue;
    }
    else if (midioption == MIDI_OPT_INVERT)
    {
        return 127. - _newmidivalue;
    }
    else if (midioption == MIDI_OPT_ROT64 || midioption == MIDI_OPT_ROT64_INV)
    {
        tempval = _prevmidivalue;
        diff = _newmidivalue - 64.;
        if (diff == -1 || diff == 1)
            diff /= 16;
        else
            diff += (diff > 0 ? -1 : +1);
        if (midioption == MIDI_OPT_ROT64)
            tempval += diff;
        else
            tempval -= diff;
        return (tempval < 0. ? 0. : (tempval > 127. ? 127.0 : tempval));
    }
    else if (midioption == MIDI_OPT_ROT64_FAST)
    {
        tempval = _prevmidivalue;
        diff = _newmidivalue - 64.;
        diff *= 1.5;
        tempval += diff;
        return (tempval < 0. ? 0. : (tempval > 127. ? 127.0 : tempval));
    }
    else if (midioption == MIDI_OPT_DIFF)
    {
        //Interpret 7-bit signed value using two's compliment.
        if (_newmidivalue >= 64.)
            _newmidivalue = _newmidivalue - 128.;
        //Apply sensitivity to signed value.
        if(sensitivity > 0)
            _newmidivalue = _newmidivalue * ((double)sensitivity / 50.);
        //Apply new value to current value.
        _newmidivalue = _prevmidivalue + _newmidivalue;
    }
    else if (midioption == MIDI_OPT_SELECTKNOB)
    {
        //Interpret 7-bit signed value using two's compliment.
        if (_newmidivalue >= 64.)
            _newmidivalue = _newmidivalue - 128.;
        //Apply sensitivity to signed value.
        if(sensitivity > 0)
            _newmidivalue = _newmidivalue * ((double)sensitivity / 50.);
        //Since this is a selection knob, we do not want to inherit previous values.
        return _newmidivalue;
    }
    else if (midioption == MIDI_OPT_BUTTON)
    {
        if (_newmidivalue != 0.) {
            _newmidivalue = !_prevmidivalue;
        } else {
            _newmidivalue = _prevmidivalue;
        }
    }
    else if (midioption == MIDI_OPT_SWITCH)
    {
        _newmidivalue = (_newmidivalue != 0);
    }
    else if (midioption == MIDI_OPT_SPREAD64)
    {
        // BJW: Spread64: Distance away from centre point (aka "relative CC")
        // Uses a similar non-linear scaling formula as ControlTTRotary::getValueFromWidget()
        // but with added sensitivity adjustment. This formula is still experimental.
        double distance = _newmidivalue - 64.;
        _newmidivalue = distance * distance * sensitivity / 50000.;
        if (distance < 0.)
            _newmidivalue = -_newmidivalue;
        // qDebug() << "Spread64: in " << distance << "  out " << _newmidivalue;
    }
    else if (midioption == MIDI_OPT_HERC_JOG)
    {
        if (_newmidivalue > 64.) { _newmidivalue -= 128.; }
		_newmidivalue += _prevmidivalue;
		//if (_prevmidivalue != 0.0) { qDebug() << "AAAAAAAAAAAA" << _prevmidivalue; }
    }
    else
    {
        qWarning("Unknown MIDI option %d", midioption);
    }

    return _newmidivalue;
}

QString ConfigValueMidi::getType() {
    if (miditype==MIDI_KEY)
        return "Key";
    else if (miditype==MIDI_CTRL)
        return "Ctrl";
    else if (miditype==MIDI_PITCH)
        return "Pitch";
    else
    	return "";
}

ConfigValueKbd::ConfigValueKbd()
{
}

ConfigValueKbd::ConfigValueKbd(QString _value) : ConfigValue(_value)
{
    QString key;

    QTextIStream(&_value) >> key;
    m_qKey = QKeySequence(key);
}

ConfigValueKbd::ConfigValueKbd(QKeySequence key)
{
    m_qKey = key;
    QTextOStream(&value) << (const QString)m_qKey;
//          qDebug() << "value" << value;
}

void ConfigValueKbd::valCopy(const ConfigValueKbd v)
{
    m_qKey = v.m_qKey;
    QTextOStream(&value) << (const QString &)m_qKey;
}

bool operator==(const ConfigValue & s1, const ConfigValue & s2)
{
    return (s1.value.toUpper() == s2.value.toUpper());
}

bool operator==(const ConfigValueMidi & s1, const ConfigValueMidi & s2)
{
    return ((s1.midichannel == s2.midichannel) && (s1.midino == s2.midino) && ( s1.miditype == s2.miditype)) ||
           ((s1.midichannel == s2.midichannel) && (s1.miditype == MIDI_PITCH) && (s1.miditype == s2.miditype));
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
}

template <class ValueType>
ConfigOption<ValueType> *ConfigObject<ValueType>::set(ConfigKey k, ValueType v)
{
    // Search for key in list, and set value if found
    ConfigOption<ValueType> *it;
    for (it = list.first(); it; it = list.next())
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
    ConfigOption<ValueType> *it;
    for (it = list.first(); it; it = list.next())
    {
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
    ConfigOption<ValueType> *it;
    for (it = list.first(); it; it = list.next())
    {
        if (QString::compare(it->val->value, v.value, Qt::CaseInsensitive) == 0){
            //qDebug() << "ConfigObject #534: QString::compare match for " << it->key->group << it->key->item;
            return it->key;
        }
        if (((ValueType)*it->val) == ((ValueType)v))
        {
            //qDebug() << "ConfigObject: match" << it->val->value.toUpper() << "with" << v.value.toUpper();
            return it->key;
        }

        if (it == list.getLast()) {
            //qDebug() << "ConfigObject: last match attempted" << it->val->value.toUpper() << "with" << v.value.toUpper();
        }
    }
    qDebug() << "No match for ConfigObject:" << v.value;
    return 0;
}

template <class ValueType>
QString ConfigObject<ValueType>::getValueString(ConfigKey k)
{
    return get(k)->val->value;
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

        while (!text.atEnd())
        {
            line = text.readLine().stripWhiteSpace();

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
                    QTextIStream(&line) >> key;
                    QString val = line.right(line.length() - key.length()); // finds the value string
                    val = val.stripWhiteSpace();
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
    // This shouldn't be done, since objects might have references to
    // members of list. Instead all member values should be set to some
    // null value.
    list.clear();
}

template <class ValueType> void ConfigObject<ValueType>::reopen(QString file)
{
    list.setAutoDelete(TRUE);

    // First try to open the config file placed in the users .mixxx directory.
    // If that fails, try the system wide CONFIG_PATH.

    filename = file;
    Parse();
}

template <class ValueType> void ConfigObject<ValueType>::Save()
{
    QFile file(filename);
#ifndef QT3_SUPPORT
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
#else
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
#endif
    {
        qDebug() << "Could not write file" << filename << ", don't worry.";
        return;
    }
    else
    {
        Q3TextStream stream(&file);

        ConfigOption<ValueType> *it;
        QString grp = "";
        for (it = list.first(); it; it = list.next())
        {
//            qDebug() << "group:" << it->key->group << "item" << it->key->item << "val" << it->val->value;
            if (it->key->group != grp)
            {
                grp = it->key->group;
                stream << "\n" << it->key->group << "\n";
            }
            stream << it->key->item << " " << it->val->value << "\n";
        }
        file.close();
        if (file.status()!=IO_Ok)
            qDebug() << "Error while writing configuration file.";
    }
}

template <class ValueType>
QString ConfigObject<ValueType>::getConfigPath()
{
    //
    // Find the config path, path where midi configuration files, skins etc. are stored.
    // On Linux the search order is whats listed in mixxx.cfg, then UNIX_SHARE_PATH
    // On Windows and Mac it is always (and only) app dir.
    //
    QString qConfigPath;
#ifdef __LINUX__
    // On Linux, check if the path is stored in the configuration database.
    if (getValueString(ConfigKey("[Config]","Path")).length()>0 && QDir(getValueString(ConfigKey("[Config]","Path"))).exists())
        qConfigPath = getValueString(ConfigKey("[Config]","Path"));
    else
    {
        // Set the path according to the compile time define, UNIX_SHARE_PATH
        qConfigPath = UNIX_SHARE_PATH;
    }
#endif
#ifdef __WIN32__
    // On Windows, set the config dir relative to the application dir
    char* str = new char[200];
    GetModuleFileName(NULL, str, 200);
    qConfigPath = QFileInfo(str).dirPath();
#endif
#ifdef __APPLE__
    // Set the path relative to the bundle directory
    CFURLRef pluginRef = CFBundleCopyBundleURL(CFBundleGetMainBundle());
    CFStringRef macPath = CFURLCopyFileSystemPath(pluginRef, kCFURLPOSIXPathStyle);
    qConfigPath = CFStringGetCStringPtr(macPath, CFStringGetSystemEncoding());
#endif
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
template class ConfigObject<ConfigValueMidi>;
template class ConfigObject<ConfigValueKbd>;

