/***************************************************************************
                          soundsource.h  -  description
                             -------------------
    begin                : Wed Feb 20 2002
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SOUNDSOURCE_H
#define SOUNDSOURCE_H

#include "defs.h"
#include <qstring.h>

/**
  *@author Tue and Ken Haste Andersen
  */

/*
  Base class for sound sources.
*/
class SoundSource {
public:
  SoundSource();
  virtual ~SoundSource();
  virtual long seek(long) = 0;
  virtual unsigned read(unsigned long size, const SAMPLE*) = 0;
  virtual long unsigned length() = 0;
  QString type;
};

#endif
