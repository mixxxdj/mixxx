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

class ConfigMIDI : ConfigObject {
 public:
    class ConfigValue : ConfigObject::ConfigValue {
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
      int   midino, midimask, midichannel;
      QString value;
    };
    
    class ConfigOption : ConfigObject::ConfigOption {
    public:
	ConfigOption(ConfigObject::ConfigKey *_key, ConfigValue *_val) : 
	    ConfigObject::ConfigOption(_key, _val->value) {};
	ConfigObject::ConfigKey *key;
    };
    
    
    ConfigMIDI(QString file) : ConfigObject(file) {};
    ~ConfigMIDI() {};
    ConfigObject::ConfigOption *get(ConfigObject::ConfigKey *key) {return ConfigObject::get(key);};
    void Save();

};

#endif
