/***************************************************************************
                          configmidi.h  -  description
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

#ifndef CONFIGMIDI_H
#define CONFIGMIDI_H

#include "configobject.h"

class ConfigMIDI : public ConfigObject {
 public:
    class ConfigValue : public ConfigObject::ConfigValue {
     public:
      ConfigValue(QString _value) : ConfigObject::ConfigValue(_value) {
        QString s2;
        QTextIStream(&_value) >> midino >> midimask >> s2 >> midichannel;
        if (s2.endsWith("h"))
            midichannel--;   // Internally midi channels are form 0-15,
        // while musicians operates on midi channels 1-16.
        else
            midichannel = 0; // Default to 0 (channel 1)
      };
      ConfigValue(int _midino, int _midimask, int _midichannel) {
	  midino = _midino;
	  midimask = _midimask;
	  midichannel = _midichannel;
	  QTextIStream(&value) << midino << midimask << "ch" << midichannel;
      }; 
      int   midino, midimask, midichannel;
    };
    
    class ConfigOption : public ConfigObject::ConfigOption {
    public:
	ConfigOption(ConfigObject::ConfigKey *_key, ConfigMIDI::ConfigValue *_val) {key=_key; val=_val;};
	ConfigObject::ConfigKey *key;
    };
    
    
    ConfigMIDI(QString file) : ConfigObject(file) {};
    ~ConfigMIDI() {};
    ConfigMIDI::ConfigOption *get(ConfigObject::ConfigKey *key) {return ConfigObject::get(key);};
    void Save();

};

#endif
