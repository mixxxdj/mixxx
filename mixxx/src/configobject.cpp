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

ConfigValueMidi::ConfigValueMidi(QString _value)
{
        QString channelMark;
        QString type;

        QTextIStream(&_value) >> type >> midino >> channelMark >> midichannel;
        if (type.contains("Key",false))
            miditype = MIDI_KEY;
        else if (type.contains("Ctrl",false))
            miditype = MIDI_CTRL;
        else
            miditype = MIDI_EMPTY;

        if (channelMark.endsWith("h"))
            midichannel--;   // Internally midi channels are form 0-15,
                             // while musicians operates on midi channels 1-16.
        else
            midichannel = 0; // Default to 0 (channel 1)

        // If empty string, default midino should be below 0.
        if (_value.length()==0)
            midino = -1;
//        qDebug("miditype: %s, midino: %i, midichannel: %i",type.latin1(),midino,midichannel);

        // Store string with corrected config value
        //value="";
        //QTextOStream(&value) << type << " " << midino << " ch " << midichannel;
}

ConfigValueMidi::ConfigValueMidi(MidiType _miditype, int _midino, int _midichannel)
{
        //qDebug("--2");
        miditype = _miditype;
        midino = _midino;
        midichannel = _midichannel;
        QTextOStream(&value) << midino << " ch " << midichannel;
        if (miditype==MIDI_KEY)
            value.prepend("Key ");
        else if (miditype==MIDI_CTRL)
            value.prepend("Ctrl ");

        //QTextIStream(&value) << midino << midimask << "ch" << midichannel;
}

void ConfigValueMidi::valCopy(const ConfigValueMidi v)
{
        //qDebug("--1, midino: %i, midimask: %i, midichannel: %i",midino,midimask,midichannel);
        miditype = v.miditype;
        midino = v.midino;
        midichannel = v.midichannel;
        QTextOStream(&value) << midino << " ch " << midichannel;
        if (miditype==MIDI_KEY)
            value.prepend("Key ");
        else if (miditype==MIDI_CTRL)
            value.prepend("Ctrl ");
        //qDebug("--1, midino: %i, midimask: %i, midichannel: %i",midino,midimask,midichannel);
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
//          qDebug("value %s",value.latin1());
}

void ConfigValueKbd::valCopy(const ConfigValueKbd v)
{
        m_qKey = v.m_qKey;
        QTextOStream(&value) << (const QString &)m_qKey;
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
        if (it->key->group == k.group & it->key->item == k.item)
        {
            //qDebug("set found. %s,%s",k.group.latin1(),k.item.latin1());
            //cout << "1: " << v.value << "\n";
            //qDebug("configobject %p",it->val);
            it->val->valCopy(v); // Should be done smarter using object copying
            //qDebug("configobject %p",it->val);
            //cout << "2: " << it->val->value << "\n";
            return it;
        }

    // If key is not found, insert it into the list of config objects
    ConfigKey *key = new ConfigKey(k.group, k.item);
    it = new ConfigOption<ValueType>(key, new ValueType(v.value));
    //qDebug("new configobject %p",it->val);
    list.append(it);
    return it;
}

template <class ValueType>
ConfigOption<ValueType> *ConfigObject<ValueType>::get(ConfigKey k)
{
    ConfigOption<ValueType> *it;
    for (it = list.first(); it; it = list.next())
    {
        //qDebug("%s %s %s %s", it->key->group.ascii(), k->group.ascii(), it->key->item.ascii(), k->item.ascii());
        if (it->key->group == k.group & it->key->item == k.item)
        {
            //cout << it->key->group << ":" << it->key->item << ", val: " << it->val->value << "\n";
            return it;
        }
    }
    // If key is not found, insert into list with null values
    ConfigKey *key = new ConfigKey(k.group, k.item);
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
//          qDebug("match %s with %s", it->val->value.upper().latin1(), v.value.upper().latin1());
        if (it->val->value.upper() == v.value.upper())
        {
            return it->key;
        }
    }
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
    if (filename.length()<1 || !configfile.open(IO_ReadOnly))
    {
        qDebug("Could not read %s",filename.latin1());
        return false;
    }
    else
    {
        // Parse the file
        int group = 0;
        QString groupStr, line;
        QTextStream text(&configfile);
        while (!text.eof())
        {
            line = text.readLine().stripWhiteSpace();

            if (line.length() != 0)
                if (line.startsWith("[") & line.endsWith("]"))
                {
                    group++;
                    groupStr = line;
                    //qDebug("Group : %s", groupStr.ascii());
                }
                else if (group>0)
                {
                    QString key;
                    QTextIStream(&line) >> key;
                    QString val = line.right(line.length() - key.length()); // finds the value string
                    val = val.stripWhiteSpace();
                    //qDebug("control: %s, value: %s",key.ascii(), val.ascii());
                    ConfigKey k(groupStr, key);
                    ValueType m(val);
                    set(k, m);
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
    if (!file.open(IO_WriteOnly| IO_Translate))
    {
        qDebug("Could not write file %s, don't worry.",filename.ascii());
        return;
    }
    else
    {
        QTextStream stream(&file);

        ConfigOption<ValueType> *it;
        QString grp = "";
        for (it = list.first(); it; it = list.next())
        {
//            qDebug("group: %s, item %s, val %s",it->key->group.latin1(), it->key->item.latin1(),it->val->value.latin1());
            if (it->key->group != grp)
            {
                grp = it->key->group;
                stream << "\n" << it->key->group << "\n";
            }
            stream << it->key->item << " " << it->val->value << "\n";
        }
        file.close();
        if (file.status()!=IO_Ok)
            qDebug("Error while writing configuration file.");
    }
}


template class ConfigObject<ConfigValue>;
template class ConfigObject<ConfigValueMidi>;
template class ConfigObject<ConfigValueKbd>;



