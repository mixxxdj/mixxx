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

#include <qobject.h>
#include "defs.h"
#include <audiofile.h>
#include <stdio.h>
#include <fstream>
#include <stdlib.h>
#include <iostream>
#include <sys/stat.h>
#include <string.h>
#include <mad.h>
#include <vector>
#include "errno.h"

/**
  *@author Tue and Ken Haste Andersen
  */

class SoundSource : public QObject {
public:
  SoundSource();
  virtual ~SoundSource();
  virtual long seek(long) = 0;
  virtual unsigned read(unsigned long size, const SAMPLE*) = 0;
  virtual long unsigned length() = 0;
};

class AFlibfile : public SoundSource {
 public:
  AFlibfile(const char*);
  ~AFlibfile();
  long seek(long);
  unsigned read(unsigned long size, const SAMPLE*);
  long unsigned length();
 private:
  int channels;
  AFfilehandle fh;
  long filelength, mp3filelength;
};

class mp3file : public SoundSource {
 public:
  mp3file(const char*);
  ~mp3file();
  long seek(long);
  unsigned read(unsigned long size, const SAMPLE*);
  long unsigned length();
 private:
  FILE *file;
  unsigned inputbuf_len;
  unsigned char *inputbuf;
  int bitrate;
  long filelength, mp3filelength;
  mad_stream Stream;
  mad_frame Frame;
  mad_synth Synth;
  vector<long> ftable,sampletable;
};


#endif
