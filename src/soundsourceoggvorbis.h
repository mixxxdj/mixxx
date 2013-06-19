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
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

class SoundSourceOggVorbis : public Mixxx::SoundSource {
 public:
  SoundSourceOggVorbis(QString qFilename);
  ~SoundSourceOggVorbis();
  int open();
  long seek(long);
  unsigned read(unsigned long size, const SAMPLE*);
  inline long unsigned length();
  int parseHeader();
  static QList<QString> supportedFileExtensions();
 private:
  int channels;
  unsigned long filelength;
  OggVorbis_File vf;
  int current_section;
};

#endif
