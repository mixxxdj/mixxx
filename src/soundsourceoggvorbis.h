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

#include <QString>
#define OV_EXCLUDE_STATIC_CALLBACKS
#include <vorbis/vorbisfile.h>

#include "soundsource.h"

class SoundSourceOggVorbis : public Mixxx::SoundSource {
 public:
  explicit SoundSourceOggVorbis(QString qFilename);
  ~SoundSourceOggVorbis();
  Result open();
  long seek(long);
  unsigned read(unsigned long size, const SAMPLE*);
  inline long unsigned length();
  Result parseHeader();
  QImage parseCoverArt();
  static QList<QString> supportedFileExtensions();
 private:
  int channels;
  unsigned long filelength;
  OggVorbis_File vf;
  int current_section;
};

#endif
