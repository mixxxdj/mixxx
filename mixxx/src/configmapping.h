/***************************************************************************
                          configmapping.h  -  description
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

#ifndef CONFIGMAPPING_H
#define CONFIGMAPPING_H

#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>

#include "configobject.h"

/**
  * Maps active ConfigFile object to MixxxApp application
  *@author Tue & Ken Haste Andersen
  */

class ConfigMapping : public QObject  {
public: 
	ConfigMapping();
	~ConfigMapping();
    QStringList *getConfigurations();
    ConfigObject *setConfiguration(const char *str);
private:
    /** List of configuration files */
    QStringList list;

};

#endif
