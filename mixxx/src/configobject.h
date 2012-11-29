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
#include <qfile.h>
#include <qkeysequence.h>
#include <qdom.h>
#include <qmap.h>
#include <QHash>

/*
typedef enum {
    MIDI_EMPTY            = 0,
    MIDI_KEY              = 1,
    MIDI_CTRL             = 2,
    MIDI_PITCH            = 3
} MidiType;
*/

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
    static ConfigKey parseCommaSeparated(QString key);
    QString group, item;
};

/* comparison function for ConfigKeys. Used by a QHash in ControlObject */
inline bool operator==(const ConfigKey &c1, const ConfigKey &c2) {
    return c1.group == c2.group && c1.item == c2.item;
}

/* QHash hash function for ConfigKey objects. */
inline uint qHash(const ConfigKey &key) {
    return qHash(key.group) ^ qHash(key.item);
}

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
    ConfigOption() { val = NULL; key = NULL;};
    ConfigOption(ConfigKey *_key, ValueType *_val) { key = _key ; val = _val; };
    virtual ~ConfigOption() {
        delete key;
        delete val;
    }
    ValueType *val;
    ConfigKey *key;
};

template <class ValueType> class ConfigObject {
  public:
    ConfigKey key;
    ValueType value;
    ConfigOption<ValueType> option;

    ConfigObject(QString file);
    ConfigObject(QDomNode node);
    ~ConfigObject();
    ConfigOption<ValueType> *set(ConfigKey, ValueType);
    ConfigOption<ValueType> *get(ConfigKey key);
    bool exists(ConfigKey key);
    ConfigKey *get(ValueType v);
    QString getValueString(ConfigKey k);
    QString getValueString(ConfigKey k, const QString& default_string);

    void clear();
    void reopen(QString file);
    void Save();
    QString getResourcePath();
    QString getSettingsPath();

  protected:
    QList< ConfigOption<ValueType>* > m_list;
    QString m_filename;

    /** Loads and parses the configuration file. Returns false if the file could
      * not be opened; otherwise true. */
    bool Parse();
};

#endif


