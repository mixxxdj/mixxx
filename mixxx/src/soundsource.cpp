/***************************************************************************
                          soundsource.cpp  -  description
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

#include "soundsource.h"

   /*
  SoundSource is an Uber-class for the reading and decoding of audio-files.
  Each class must have the following member functions:
    initializer with a filename
    seek()
    read()
    length()
*/
SoundSource::SoundSource()
{
    SRATE = 0;
}

SoundSource::~SoundSource()
{
}

int SoundSource::getSrate()
{
    return SRATE;
}

