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
#include <q3ptrlist.h>
#include <q3valuelist.h>
#include <qfile.h>
#include <q3textstream.h>
#include <qkeysequence.h>
#include <qdom.h>
#include <qmap.h>

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
    MIDI_OPT_DIFF             = 5,
    MIDI_OPT_BUTTON           = 6, // Button Down (!=00) and Button Up (00) events happen together
    MIDI_OPT_SWITCH           = 7, // Button Down (!=00) and Button Up (00) events happen seperately
    MIDI_OPT_HERC_JOG         = 8, // Generic hercules wierd range correction
    MIDI_OPT_SPREAD64         = 9, // Accelerated difference from 64
    MIDI_OPT_SELECTKNOB       = 10,// Relative knob which can be turned forever and outputs a signed value.
} MidiOption;

typedef QMap<char,char> MidiValueMap;

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
    inline ConfigValue(QDomNode /* node */) { qFatal("ConfigValue from QDomNode not implemented here"); }
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
    ConfigValueMidi(QDomNode node);
    void valCopy(const ConfigValueMidi v);
    char translateValue(char value);
    double ComputeValue(MidiType _miditype, double _prevmidivalue, double _newmidivalue);
    friend bool operator==(const ConfigValueMidi & s1, const ConfigValueMidi & s2);

    MidiType miditype;
    int midino, midichannel;
    unsigned int sensitivity;
    MidiOption midioption;
    // BJW: Static translation of MIDI values for this object, defined in the mapping file.
    MidiValueMap translateMidiValues;
};

class ConfigValueKbd : public ConfigValue
{
public:
    ConfigValueKbd();
    ConfigValueKbd(QString _value);
    ConfigValueKbd(QKeySequence key);
    inline ConfigValueKbd(QDomNode /* node */) { qFatal("ConfigValueKbd from QDomNode not implemented here"); }
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
	ConfigObject(QDomNode node);
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
    Q3PtrList< ConfigOption<ValueType> > list;
    QString filename;

    /** Loads and parses the configuration file. Returns false if the file could
      * not be opened; otherwise true. */
    bool Parse();
};

#endif


