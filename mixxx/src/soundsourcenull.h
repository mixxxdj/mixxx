/***************************************************************************
                          soundsourcenull.h  -  description
                             -------------------
    begin                : Tue Feb 26 2002
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

#ifndef SOUNDSOURCENULL_H
#define SOUNDSOURCENULL_H

#include <soundsource.h>

/**
  *@author Tue and Ken Haste Andersen
  */

class SoundSourceNull : public SoundSource  {
public: 
	SoundSourceNull();
	~SoundSourceNull();
	long seek(long);
	unsigned read(unsigned long size, const SAMPLE*);
	long unsigned length();
};

#endif
