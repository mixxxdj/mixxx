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
        QString line = QTextIStream(handle).readLine();

        QString s;
        QTextIStream((const QString*)&line) >> s;
        if (s.startsWith("[") & s.endsWith("]"))
        {
            group++;
            groupStr = s;
        }
        else if (group>0)
        {
            int no, mask, ch;
            QString s2;
            QTextIStream((const QString*)&line) >> s >> no >> mask >> s2 >> ch;
            if (s2.endsWith("h"))
                ch--;   // Internally midi channels are form 0-15,
                        // while musicians operates on midi channels 1-16.
            else
                ch = 0; // Default to 0 (channel 1)

            //qDebug("control: %s, no: %i, mask: %i, str: %s, ch: %i",s.ascii(), no,mask,s2.ascii(),ch);
            ConfigObject::ConfigKey k(groupStr, s);
            ConfigObject::ConfigValue m(no, mask, ch);
            cfg->set(&k, &m);


        }
    }
    return cfg;
}
