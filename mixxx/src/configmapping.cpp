/***************************************************************************
                          configmapping.cpp  -  description
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

#include "configmapping.h"
#include <qdir.h>

ConfigMapping::ConfigMapping()
{
    QDir d("config");
    d.setSorting(QDir::Name);
    list = d.entryList("*.cfg");
}

ConfigMapping::~ConfigMapping()
{
}

QStringList *ConfigMapping::getConfigurations()
{
    return &list;
}

ConfigObject *ConfigMapping::setConfiguration(const char *str)
{
    // Construct config object
    ConfigObject *cfg = new ConfigObject();

    // Open file for reading
    QString filename("config/");
    filename.append(str);
    FILE *handle = fopen(filename.ascii(),"r");
    if (handle == 0)
        qFatal("Could not open file %s",filename.ascii());

    // Parse the file
    int group = 0;
    QString groupStr;
    while (!QTextIStream(handle).atEnd())
    {
        QString s;
        QTextIStream(handle) >> s;
        if (s.startsWith("[") & s.endsWith("]"))
        {
            group++;
            groupStr = s;
        }
        else if (group>0)
        {
            int no, mask, ch;
            QString s2;
            QTextIStream(handle) >> no >> mask >> s2 >> ch;
            if (!s2.endsWith("h"))
                ch = 0;

            //qDebug("no: %i, mask: %i, str: %s, ch: %i",no,mask,s2.ascii(),ch);
            ConfigObject::ConfigKey k(groupStr, s);
            ConfigObject::ConfigValue m(no, mask, ch);
            cfg->set(&k, &m);


        }
    }
    return cfg;
}
