/***************************************************************************
                          soundsourceoggvorbis.h  -  ogg vorbis decoder
                             -------------------
    copyright            : (C) 2003 by Svein Magne Bang
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

#ifndef SOUNDSOURCEOGGVORBIS_H
#define SOUNDSOURCEOGGVORBIS_H

#include <qstring.h>
#include "soundsource.h"
#include "trackinfoobject.h"
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

class TrackInfoObject;


class SoundSourceOggVorbis : public SoundSource {
 public:
  SoundSourceOggVorbis(QString qFilename);
  ~SoundSourceOggVorbis();
  long seek(long);
  unsigned read(unsigned long size, const SAMPLE*);
  inline long unsigned length();
  static int ParseHeader( TrackInfoObject * );
 private:
  int channels;
  unsigned long filelength;
  FILE *vorbisfile;
  OggVorbis_File vf;
  int current_section;
  unsigned long ret, needed, index;
  SAMPLE* dest;
};

#endif
