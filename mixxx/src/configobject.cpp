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

ConfigObject::ConfigObject(QString file)
{
    filename = "config/" + file;
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
            it->val->value = v->value;      // Should be done smarter using object copying
            return it;
        }

    // If key is not found, insert it into the list of config objects
    ConfigObject::ConfigKey *key = new ConfigObject::ConfigKey(k->group, k->control);
    it = new ConfigObject::ConfigOption(key, new ConfigObject::ConfigValue(v->value));
    list.append(it);
    return it;
}

ConfigObject::ConfigOption *ConfigObject::get(ConfigKey *k)
{
    ConfigObject::ConfigOption *it;
    for (it = list.first(); it; it = list.next())
        if (it->key->group == k->group & it->key->control == k->control)
            return it;

    // If key is not found, insert into list with null values
    ConfigObject::ConfigKey *key = new ConfigObject::ConfigKey(k->group, k->control);
    it = new ConfigObject::ConfigOption(key, new ConfigObject::ConfigValue(""));
    list.append(it);
    return it;
}

void ConfigObject::Parse()
{
    // Open file for reading
    FILE *handle = fopen(filename.ascii(),"r");
    if (handle == 0)
        qDebug("Could not open file %s",filename.ascii());
    else
    {
        // Parse the file
        int group = 0;
        QString groupStr;
        while (!QTextIStream(handle).atEnd())
        {
            QString line = QTextIStream(handle).readLine();

            if (line.startsWith("[") & line.endsWith("]"))
            {
                group++;
                groupStr = line;
		qDebug("Group : %s", line.ascii());
            }
            else if (group>0)
            {
		QString key = line.left(line.find(" ")); // finds the key
		QString val = line.right(line.length() - key.length()); // finds the value string
                qDebug("control: %s, value: %s",key.ascii(), val.ascii());
                ConfigObject::ConfigKey k(groupStr, key);
                ConfigObject::ConfigValue m(val);
                set(&k, &m);
            }
        }
    }
    fclose(handle);
}

void ConfigObject::Save() 
{
    FILE *handle = fopen(filename.ascii(),"w");
    if (handle == 0)
        qDebug("Could not open file %s",filename.ascii());

    ConfigObject::ConfigOption *it;
    QString grp = list.first()->key->group;
    for (it = list.first(); it; it = list.next())
	if (it->key->group != grp) 
	{
	    grp = it->key->group;
	    fprintf(handle, "[%s]\n", it->key->group.ascii());
	} else
	    fprintf(handle, "%s %s\n", it->key->control.ascii(), it->val->value.ascii());    

    fclose(handle);
}

