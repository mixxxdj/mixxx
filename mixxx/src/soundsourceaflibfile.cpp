#ifndef Q_WS_WIN
#include "soundsourceaflibfile.h"

/*
  Class for reading files using libaudiofile
*/
SoundSourceAFlibfile::SoundSourceAFlibfile(const char* filename) {
  fh = afOpenFile(filename,"r",0);
  if (fh == AF_NULL_FILEHANDLE) {
    cout << "Error opening file.\n";
    exit(1);
  }
  channels = 2;
  filelength = 2*afGetFrameCount(fh,AF_DEFAULT_TRACK);
}

SoundSourceAFlibfile::~SoundSourceAFlibfile() {
  afCloseFile(fh);
};

long SoundSourceAFlibfile::seek(long filepos) {
  afSeekFrame(fh, AF_DEFAULT_TRACK, (AFframecount) (filepos/channels));
  return filepos;
}
/*
  read <size> samples into <destination>, and return the number of
  samples actually read.
*/
unsigned SoundSourceAFlibfile::read(unsigned long size, const SAMPLE* destination) {
  return afReadFrames(fh,AF_DEFAULT_TRACK, (SAMPLE *)destination
		      ,size/channels)*channels;
}
/*
  Return the length of the file in samples.
*/
long unsigned SoundSourceAFlibfile::length() {
  return filelength;
}
#endif
