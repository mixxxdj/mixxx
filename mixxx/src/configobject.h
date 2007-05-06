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
    MIDI_EMPTY            = 0,
    MIDI_KEY              = 1,
    MIDI_CTRL             = 2,
    MIDI_PITCH            = 3
} MidiType;

typedef enum {
    MIDI_OPT_NORMAL           = 0,
    MIDI_OPT_INVERT           = 1,
    MIDI_OPT_ROT64            = 2,
    MIDI_OPT_ROT64_INV        = 3,
    MIDI_OPT_ROT64_FAST       = 4,
	MIDI_OPT_DIFF			  = 5
} MidiOption;

/*
  Class for the key for a specific configuration element. A key consists of a
  group and an item.
*/
class ConfigKey
{
public:
    ConfigKey();
    ConfigKey(QString g, QString i);

    QString group, item;
};

/*
  The value corresponding to a key. The basic value is a string, but can be
  subclassed to more specific needs.
*/
class ConfigValue
{
public:
    ConfigValue();
    ConfigValue(QString _value);
    ConfigValue(int _value);
    void valCopy(const ConfigValue _value);

    QString value;
    friend bool operator==(const ConfigValue & s1, const ConfigValue & s2);
};

class ConfigValueMidi : public ConfigValue
{
public:
    ConfigValueMidi();
    ConfigValueMidi(QString _value);
    ConfigValueMidi(MidiType _miditype, int _midino, int _midichannel);
    void valCopy(const ConfigValueMidi v);
    double ComputeValue(MidiType _miditype, double _prevmidivalue, double _newmidivalue);
    friend bool operator==(const ConfigValueMidi & s1, const ConfigValueMidi & s2);

    MidiType miditype;
    int midino, midichannel;
    MidiOption midioption;
};

class ConfigValueKbd : public ConfigValue
{
public:
    ConfigValueKbd();
    ConfigValueKbd(QString _value);
    ConfigValueKbd(QKeySequence key);
    void valCopy(const ConfigValueKbd v);
    friend bool operator==(const ConfigValueKbd & s1, const ConfigValueKbd & s2);

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
    ConfigKey *get(ValueType v);
    QString getValueString(ConfigKey k);

    void clear();
    void reopen(QString file);
    void Save();
    QString getConfigPath();

  protected:
    QPtrList< ConfigOption<ValueType> > list;
    QString filename;

    /** Loads and parses the configuration file. Returns false if the file could
      * not be opened; otherwise true. */
    bool Parse();
};

#endif

