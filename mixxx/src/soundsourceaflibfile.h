
#ifdef Q_WS_X11

#include "soundsource.h"
#include <audiofile.h>
#include <stdio.h>

class SoundSourceAFlibfile : public SoundSource {
 public:
  SoundSourceAFlibfile(const char*);
  ~SoundSourceAFlibfile();
  long seek(long);
  unsigned read(unsigned long size, const SAMPLE*);
  long unsigned length();
 private:
  int channels;
  AFfilehandle fh;
  unsigned long filelength;
};
#endif

