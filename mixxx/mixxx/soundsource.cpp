/***************************************************************************
                          soundsource.cpp  -  description
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

#include "soundsource.h"

   /*
  SoundSource is a Uber-class for the reading and decoding of audio-files.
  Each class must have the following member functions:
    initializer with a filename
    seek()
    read()
    length()
*/
SoundSource::SoundSource() {}
SoundSource::~SoundSource() {}
/*
  Class for reading files using libaudiofile
*/
AFlibfile::AFlibfile(const char* filename) {
  fh = afOpenFile(filename,"r",0);
  if (fh == AF_NULL_FILEHANDLE) {
    cout << "Error opening file.\n";
    exit(1);
  }
  channels = 2;
  filelength = 2*afGetFrameCount(fh,AF_DEFAULT_TRACK);
}

AFlibfile::~AFlibfile() {
  afCloseFile(fh);
};

long AFlibfile::seek(long filepos) {
  afSeekFrame(fh, AF_DEFAULT_TRACK, (AFframecount) (filepos/channels));
  return filepos;
}
/*
  read <size> samples into <destination>, and return the number of
  samples actually read.
*/
unsigned AFlibfile::read(unsigned long size, const SAMPLE* destination) {
  return afReadFrames(fh,AF_DEFAULT_TRACK, (SAMPLE *)destination
		      ,size/channels)*channels;
}
/*
  Return the length of the file in samples.
*/
long unsigned AFlibfile::length() {
  return filelength;
}

/*
  Reading and decoding of mp3-files:
*/
mp3file::mp3file(const char* filename) {
  // Open the file:
  file = fopen(filename,"r");
  if (!file) {
	cout << "Open of " << filename << " failed\n" << flush;
	exit(-1);
  }
  // Read in the whole file into inputbuf:
  struct stat filestat;
  stat(filename, &filestat);
  mp3filelength = filestat.st_size;
  inputbuf_len = mp3filelength;
  inputbuf = new unsigned char[inputbuf_len];
	if (fread(inputbuf,1,mp3filelength,file) != mp3filelength)
	  cout << "Error reading mp3-file.\n" << flush;
	// Transfer it to the mad stream-buffer:
  mad_stream_init(&Stream);
  mad_stream_buffer(&Stream, inputbuf, mp3filelength);
  /*
    Read and decode the header:
  */
  mad_header Header;
  mad_header_init(&Header);
  // Make a table of the frames:
  long total_bytes = 0;
  while (mad_header_decode(&Header, &Stream) == 0) {
    ftable.push_back((long)(Stream.this_frame - Stream.buffer));
    sampletable.push_back(total_bytes/2);
    bitrate = Header.bitrate/1000;
    total_bytes +=
 (Stream.next_frame-Stream.this_frame)*SRATE*8*4/(bitrate*1000);
    cout <<
 ftable[ftable.size()-1]<<":"<<sampletable[ftable.size()-1]<<":"<<bitrate << "..
 ";
  }
  total_bytes +=
    (mp3filelength -
 (Stream.this_frame-Stream.buffer))*SRATE*8*4/(bitrate*1000);
  // Calc the length of the file:
  filelength = total_bytes/2; // filelength is measured in samples.
  mad_header_finish(&Header);
  // Re-init buffer:
  mad_stream_finish(&Stream);
  mad_stream_init(&Stream);
  mad_stream_buffer(&Stream, inputbuf, mp3filelength);
  cout << filelength << ":" << bitrate <<"\n";
}

mp3file::~mp3file() {
  fclose(file);
  delete inputbuf;
  mad_stream_finish(&Stream);
  mad_frame_finish(&Frame);
  mad_synth_finish(&Synth);
}
/*
   Seek towards filepos (in samples). Return the position which was actually
   found.
*/
long mp3file::seek(long filepos) {
  int i;
  for (i=0; (i<ftable.size()) && (sampletable[i]<filepos); i++);
  // Re-init buffer:
  mad_stream_finish(&Stream);
  mad_stream_init(&Stream);
  mad_stream_buffer(&Stream, inputbuf+ftable[i], mp3filelength-ftable[i]);
  cout << ftable[i] << "\n";
  return sampletable[i];
}

/*
  Read <samples_wanted> samples into the buffer <destination>.
*/
unsigned mp3file::read(unsigned long samples_wanted, const SAMPLE* _destination)
 {
  SAMPLE *destination = (SAMPLE*)_destination;
  unsigned Total_samples_decoded = 0;

  while (Total_samples_decoded < samples_wanted) {
    cout << Total_samples_decoded << ",";
    if(mad_frame_decode(&Frame,&Stream))
      if(MAD_RECOVERABLE(Stream.error))
	{
	  fprintf(stderr,"Recoverable frame level error (%s)\n",
	    mad_stream_errorstr(&Stream));
	  fflush(stderr);
	  continue;
	}
      else
	if(Stream.error==MAD_ERROR_BUFLEN)
	  continue;
	else
	  {
	    fprintf(stderr,"Unrecoverable frame level error (%s).\n",
		    mad_stream_errorstr(&Stream));
	    break;
	  }
    /* Once decoded the frame is synthesized to PCM samples. No errors
     * are reported by mad_synth_frame();
     */
    mad_synth_frame(&Synth,&Frame);
    /* Synthesized samples must be converted from mad's fixed
     * point number to the consumer format. Here we use unsigned
     * 16 bit big endian integers on two channels. Integer samples
     * are temporarily stored in a buffer that is flushed when
     * full.
     */
    for(int i=0;i<Synth.pcm.length;i++)
      {
	unsigned short	Sample;
	/* Left channel */
	Sample=(SAMPLE)(Synth.pcm.samples[0][i]>>(MAD_F_FRACBITS-15));
	*(destination++) = Sample;
	/* Right channel. If the decoded stream is monophonic then
	 * the right output channel is the same as the left one.
	 */
	if(MAD_NCHANNELS(&Frame.header)==2)
	  Sample=(SAMPLE)(Synth.pcm.samples[0][i]>>(MAD_F_FRACBITS-15));
	*(destination++) = Sample;
      }
    Total_samples_decoded += 2*Synth.pcm.length;
  }

  cout << "decoded " << Total_samples_decoded << "\n" << flush;

  return Total_samples_decoded;
}

long unsigned mp3file::length() {
    return filelength;
}
