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

#ifdef __MP4V2__
    #include <mp4v2/mp4v2.h>
#else
    #include <mp4.h>
#endif

#include <neaacdec.h>
#include <QString>
#include "soundsource.h"
#include "m4a/ip.h"

//As per QLibrary docs: http://doc.trolltech.com/4.6/qlibrary.html#resolve
#ifdef Q_WS_WIN
#define MY_EXPORT __declspec(dllexport)
#else
#define MY_EXPORT
#endif

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

extern "C" MY_EXPORT SoundSource* getSoundSource(QString filename)
{
    return new SoundSourceM4A(filename);
}

extern "C" MY_EXPORT int ParseHeader(TrackInfoObject* track)
{
    return SoundSourceM4A::ParseHeader(track);
}

#endif
