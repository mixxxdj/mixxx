/***************************************************************************
                          configobject.h  -  description
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

#ifndef CONFIGOBJECT_H
#define CONFIGOBJECT_H

#include <qstring.h>
#include <qptrlist.h>
#include <qvaluelist.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qkeysequence.h>

typedef enum {
    EMPTY            = 0,
    KEY              = 1,
    CTRL             = 2
} MidiType;


/*
  Class for the key for a specific configuration element. A key consists of a
  group and an item.
*/
class ConfigKey {
 public:
    ConfigKey() {};
    ConfigKey(QString g, QString i) { group = g; item = i; };
    QString group, item;
};

/*
  The value corresponding to a key. The basic value is a string, but can be
  subclassed to more specific needs.
*/
class ConfigValue
{
  public:
    ConfigValue() {};
    ConfigValue(QString _value) {value = _value;};
    ConfigValue(int _value) { value = QString::number(_value); };
    void valCopy(const ConfigValue _value) { value = _value.value; };
    QString value;
};

class ConfigValueMidi : public ConfigValue
{
public:
    ConfigValueMidi() {};
    ConfigValueMidi(QString _value) : ConfigValue(_value)
    {
        QString channelMark;
        QString type;

        QTextIStream(&_value) >> type >> midino >> channelMark >> midichannel;
        if (type.contains("Key",false))
            miditype = KEY;
        else if (type.contains("Ctrl",false))
            miditype = CTRL;
        else
            miditype = EMPTY;

        if (channelMark.endsWith("h"))
            midichannel--;   // Internally midi channels are form 0-15,
                             // while musicians operates on midi channels 1-16.
        else
            midichannel = 0; // Default to 0 (channel 1)

        // If empty string, default midino should be below 0.
        if (_value.length()==0)
            midino = -1;
//        qDebug("miditype: %s, midino: %i, midichannel: %i",type.latin1(),midino,midichannel);
    };

    ConfigValueMidi(MidiType _miditype, int _midino, int _midichannel)
    {
        //qDebug("--2");
        miditype = _miditype;
        midino = _midino;
        midichannel = _midichannel;
        QTextOStream(&value) << midino << " ch " << midichannel;
        if (miditype==KEY)
            value.prepend("Key ");
        else if (miditype==CTRL)
            value.prepend("Ctrl ");

        //QTextIStream(&value) << midino << midimask << "ch" << midichannel;
    };
    void valCopy(const ConfigValueMidi v)
    {
        //qDebug("--1, midino: %i, midimask: %i, midichannel: %i",midino,midimask,midichannel);
        miditype = v.miditype;
        midino = v.midino;
        midichannel = v.midichannel;
        QTextOStream(&value) << midino << " ch " << midichannel;
        if (miditype==KEY)
            value.prepend("Key ");
        else if (miditype==CTRL)
            value.prepend("Ctrl ");
        //qDebug("--1, midino: %i, midimask: %i, midichannel: %i",midino,midimask,midichannel);
    }

    MidiType miditype;
    int midino, midichannel;
};

class ConfigValueKbd : public ConfigValue
{
public:
    ConfigValueKbd() {};
    ConfigValueKbd(QString _value) : ConfigValue(_value)
    {
        QString key;

        QTextIStream(&_value) >> key;
        m_qKey = QKeySequence(key);
    };

    ConfigValueKbd(QKeySequence key)
    {
        m_qKey = key;
        QTextOStream(&value) << (const QString &)m_qKey;
    };

    void valCopy(const ConfigValueKbd v)
    {
        m_qKey = v.m_qKey;
        QTextOStream(&value) << (const QString &)m_qKey;
    }
    
    QKeySequence m_qKey;
};

template <class ValueType> class ConfigOption
{
  public:
    ConfigOption() {};
    ConfigOption(ConfigKey *_key, ValueType *_val) { key = _key ; val = _val; };
    ValueType *val;
    ConfigKey *key;
};

template <class ValueType> class ConfigObject
{
  public:
    ConfigKey key;
    ValueType value;
    ConfigOption<ValueType> option;
    
    ConfigObject(QString file);
    ~ConfigObject();
    ConfigOption<ValueType> *set(ConfigKey, ValueType);
    ConfigOption<ValueType> *get(ConfigKey key);
    QString getValueString(ConfigKey k);

    void clear();
    void reopen(QString file);
    void Save();
    
  protected:
    QPtrList< ConfigOption<ValueType> > list;
    QString filename;

    /** Loads and parses the configuration file. Returns false if the file could
      * not be opened; otherwise true. */
    bool Parse();
};

#endif

