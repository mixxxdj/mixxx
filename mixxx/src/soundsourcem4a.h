/***************************************************************************
                          soundsourcem4a.h  -  mp4/m4a decoder
                             -------------------
    copyright            : (C) 2008 by Garth Dahlstrom
    email                : ironstorm@users.sf.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SOUNDSOURCEM4A_H
#define SOUNDSOURCEM4A_H

#include <mp4.h>
#include <neaacdec.h>

#include <QString>

#include "soundsource.h"

#include "m4a/ip.h"

class TrackInfoObject;

class SoundSourceM4A : public SoundSource {
 public:
  SoundSourceM4A(QString qFileName);
  ~SoundSourceM4A();
  long seek(long);
  unsigned read(unsigned long size, const SAMPLE*);
  inline long unsigned length();
  static int ParseHeader( TrackInfoObject * );
 private:
  int channels;
  int trackId;
  unsigned long filelength;
  MP4FileHandle mp4file;
  input_plugin_data ipd;
};

#endif
