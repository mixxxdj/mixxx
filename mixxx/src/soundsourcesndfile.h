#ifndef SOUNDSOURCESNDFILE_H
#define SOUNDSOURCESNDFILE_H

#include "soundsource.h"
#include <stdio.h>
#include <sndfile.h>

class SoundSourceSndFile : public SoundSource {
 public:
  SoundSourceSndFile(const char*);
  ~SoundSourceSndFile();
  long seek(long);
  unsigned read(unsigned long size, const SAMPLE*);
  inline long unsigned length();
 private:
  int channels;
  SNDFILE *fh;
  SF_INFO *info;
  unsigned long filelength;
};

#endif