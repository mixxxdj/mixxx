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

/**
  * List element of configuration values as stored in a specific configuration file
  *@author Tue & Ken Haste Andersen
  */

class ConfigObject {
public: 
    class ConfigKey {
     public:
      ConfigKey(QString g, QString c) { group = g; control = c; };
      QString group, control;
    };

    class ConfigValue {
     public:
      ConfigValue(int no, int mask, int channel) { midino = no; midimask = mask; midichannel = channel; };
      int   midino, midimask, midichannel;
    };

    class ConfigOption {
     public:
      ConfigOption(ConfigKey *_key, ConfigValue *_val) { key = _key ; val = _val; };
      ConfigValue *val;
      ConfigKey *key;
    };

    ConfigObject();
    ~ConfigObject();
    ConfigObject::ConfigOption *set(ConfigObject::ConfigKey *key, ConfigObject::ConfigValue *val);
    ConfigObject::ConfigOption *get(ConfigObject::ConfigKey *key);
private:
    QPtrList<ConfigObject::ConfigOption> list;
};

#endif
