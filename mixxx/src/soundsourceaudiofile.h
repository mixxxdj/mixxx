/***************************************************************************
                          soundsourceaudiofile.h  -  description
                             -------------------
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

#ifndef SOUNDSOURCEAUDIOFILE_H
#define SOUNDSOURCEAUDIOFILE_H

#include "soundsource.h"
#include <audiofile.h>
#include <stdio.h>

class TrackInfoObject;

/**
  * Class for reading files using libaudiofile
  */
class SoundSourceAudioFile : public SoundSource {
public:
  SoundSourceAudioFile(QString qFilename);
  ~SoundSourceAudioFile();
  long seek(long);
  unsigned read(unsigned long size, const SAMPLE*);
  inline long unsigned length();
  static int ParseHeader( TrackInfoObject * );
private:
  int channels;
  AFfilehandle fh;
  unsigned long filelength;
  SAMPLE *buffer;
};

#endif

