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
#include "defs.h"

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
class ConfigValue {
 public:
    ConfigValue() {};
    ConfigValue(QString _value) {value = _value;};
    QString value;
};

class ConfigValueMidi : public ConfigValue {
 public:
    ConfigValueMidi() {};
    ConfigValueMidi(QString _value) : ConfigValue(_value) {
	QString s2;
	QTextIStream(&_value) >> midino >> midimask >> s2 >> midichannel;
	if (s2.endsWith("h"))
	    midichannel--;   // Internally midi channels are form 0-15,
	// while musicians operates on midi channels 1-16.
	else
	    midichannel = 0; // Default to 0 (channel 1)
    };
    ConfigValueMidi(int _midino, int _midimask, int _midichannel) {
	midino = _midino;
	midimask = _midimask;
	midichannel = _midichannel;
	QTextIStream(&value) << midino << midimask << "ch" << midichannel;
    }; 
    int   midino, midimask, midichannel;
};

template <class ValueType> class ConfigOption {
 public:
    ConfigOption() {};
    ConfigOption(ConfigKey *_key, ValueType *_val) { key = _key ; val = _val; };
    ValueType *val;
    ConfigKey *key;
};

template <class ValueType> class ConfigObject {
 public: 
    ConfigKey key;
    ValueType value;
    ConfigOption<ValueType> option;
    
    ConfigObject(QString file);
    ~ConfigObject();
    ConfigOption<ValueType> *set(ConfigKey, ValueType);
    ConfigOption<ValueType> *get(ConfigKey key);
    QString getValueString(ConfigKey k);

    void Save();
    
 protected:
    QPtrList< ConfigOption<ValueType> > list;
    QString filename;
    
    void Parse();
};

#endif

