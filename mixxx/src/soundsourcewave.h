#ifndef SOUNDSOURCEWAVE_H
#define SOUNDSOURCEWAVE_H

#include "soundsource.h"
#include <stdio.h>
#include <sndfile.h>

class SoundSourceWave : public SoundSource {
 public:
  SoundSourceWave(const char*);
  ~SoundSourceWave();
  long seek(long);
  unsigned read(unsigned long size, const SAMPLE*);
  long unsigned length();
 private:
  int channels;
  SNDFILE *fh;
  SF_INFO *info;
  unsigned long filelength;
};

#endif