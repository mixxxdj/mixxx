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

template <class ValueType> ConfigObject<ValueType>::ConfigObject(QString file)
{
    filename = "config/" + file;
    list.setAutoDelete(TRUE);
    Parse();
}

template <class ValueType> ConfigObject<ValueType>::~ConfigObject()
{
}

template <class ValueType> 
ConfigOption<ValueType> *ConfigObject<ValueType>::set(ConfigKey k, ValueType v)
{
    // Search for key in list, and set value if found
    ConfigOption<ValueType> *it;
    for (it = list.first(); it; it = list.next())
        if (it->key->group == k.group & it->key->item == k.item)
        {
            it->val->value = v.value;      // Should be done smarter using object copying
            return it;
        }

    // If key is not found, insert it into the list of config objects
    ConfigKey *key = new ConfigKey(k.group, k.item);
    it = new ConfigOption<ValueType>(key, new ValueType(v.value));
    list.append(it);
    return it;
}

template <class ValueType> 
ConfigOption<ValueType> *ConfigObject<ValueType>::get(ConfigKey k)
{
    ConfigOption<ValueType> *it;
    for (it = list.first(); it; it = list.next())
    {
	//qDebug("%s %s %s %s", it->key->group.ascii(), k->group.ascii(), it->key->item.ascii(), k->item.ascii());
        if (it->key->group == k.group & it->key->item == k.item)
            return it;
    }
    // If key is not found, insert into list with null values
    ConfigKey *key = new ConfigKey(k.group, k.item);
    it = new ConfigOption<ValueType>(key, new ValueType(""));
    list.append(it);
    return it;
}

template <class ValueType> 
QString ConfigObject<ValueType>::getValueString(ConfigKey k)
{
    return get(k)->val->value;
}

template <class ValueType> void ConfigObject<ValueType>::Parse()
{
    // Open file for reading
    FILE *handle = fopen(filename.ascii(),"r");
    if (handle == 0)
        qDebug("Could not open file %s",filename.ascii());
    else
    {
        // Parse the file
        int group = 0;
        QString groupStr, line;
        while (!QTextIStream(handle).atEnd())
        {
	    line = QTextIStream(handle).readLine().stripWhiteSpace();
	    
	    if (line.length() != 0) 
		if (line.startsWith("[") & line.endsWith("]"))
		{
		    group++;
		    groupStr = line;
		    //qDebug("Group : %s", groupStr.ascii());
		}
		else if (group>0)
		{ 
		    QString key;
		    QTextIStream(&line) >> key;
		    QString val = line.right(line.length() - key.length()); // finds the value string
		    val = val.stripWhiteSpace();
		    //qDebug("control: %s, value: %s",key.ascii(), val.ascii());
		    ConfigKey k(groupStr, key);
		    ValueType m(val);
		    set(k, m);
		}
        }
    }
    fclose(handle);
}

template <class ValueType> void ConfigObject<ValueType>::Save() 
{
    FILE *handle = fopen(filename.ascii(),"w");
    if (handle == 0)
        qDebug("Could not open file %s",filename.ascii());

    ConfigOption<ValueType> *it;
    QString grp = 0;
    for (it = list.first(); it; it = list.next())
    {
	if (it->key->group != grp) 
	{
	    grp = it->key->group;
	    fprintf(handle, "\n%s\n", it->key->group.ascii());
	}
	fprintf(handle, "%s %s\n", it->key->item.ascii(), it->val->value.ascii());    
    }

    fclose(handle);
}

template class ConfigObject<ConfigValue>;
template class ConfigObject<ConfigValueMidi>;



