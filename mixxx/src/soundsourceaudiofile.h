#ifndef SOUNDSOURCEAUDIOFILE_H
#define SOUNDSOURCEAUDIOFILE_H

#include "soundsource.h"
#include <audiofile.h>
#include <stdio.h>

class SoundSourceAudioFile : public SoundSource {
 public:
  SoundSourceAudioFile(const char*);
  ~SoundSourceAudioFile();
  long seek(long);
  unsigned read(unsigned long size, const SAMPLE*);
  long unsigned length();
 private:
  int channels;
  AFfilehandle fh;
  unsigned long filelength;
};

#endif

