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
  In addition there must be a static member:
    int ParseHeader(TrackInfoObject *Track)
  which is used for parsing header information, like trackname,length etc. The
  return type is int: 0 for OK, -1 for an error.
*/
SoundSource::SoundSource(QString qFilename)
{
    m_qFilename = qFilename;
    SRATE = 0;    
}

SoundSource::~SoundSource()
{
}

int SoundSource::getSrate()
{
    return SRATE;
}

QPtrList<long unsigned int> *SoundSource::getCuePoints()
{
    return 0;
}

QString SoundSource::getFilename()
{
    return m_qFilename;
}
