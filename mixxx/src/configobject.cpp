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

ConfigObject::ConfigObject()
{
    list.setAutoDelete(TRUE);
}

ConfigObject::~ConfigObject()
{
}

ConfigObject::ConfigOption *ConfigObject::set(ConfigKey *k, ConfigValue *v)
{
    // Search for key in list, and set value if found
    ConfigObject::ConfigOption *it;
    for (it = list.first(); it; it = list.next())
        if (it->key->group == k->group & it->key->control == k->control)
        {
            it->val->midino = v->midino;      // Should be done smarter using object copying
            it->val->midimask = v->midimask;
            return it;
        }

    // If key is not found, insert it into the list of config objects
    ConfigObject::ConfigKey *key = new ConfigObject::ConfigKey(k->group, k->control);
    it = new ConfigObject::ConfigOption(key, new ConfigObject::ConfigValue(v->midino, v->midimask));
    list.append(it);
    return it;
}

ConfigObject::ConfigOption *ConfigObject::get(ConfigKey *k)
{
    ConfigObject::ConfigOption *it;
    for (it = list.first(); it; it = list.next())
        if (it->key->group == k->group & it->key->control == k->control)
            return it;

    // If key is not found, insert into list with null value
    ConfigObject::ConfigKey *key = new ConfigObject::ConfigKey(k->group, k->control);
    it = new ConfigObject::ConfigOption(key, new ConfigObject::ConfigValue(0,0));
    list.append(it);
    return it;
}
