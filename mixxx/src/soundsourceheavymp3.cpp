#include "soundsourceheavymp3.h"
#include <iostream>
#include <cstdlib>
SoundSourceHeavymp3::SoundSourceHeavymp3(const char *filename) {
  // Open the file:
  file = fopen(filename,"r");
  if (!file) {
      qFatal("Open of %s failed",filename);
  }
  // Read in the whole file into inputbuf:
  struct stat filestat;
  stat(filename, &filestat);
  unsigned int mp3filelength = filestat.st_size;
  inputbuf_len = mp3filelength;
  inputbuf = new unsigned char[inputbuf_len];
  if (fread(inputbuf,1,mp3filelength,file) != mp3filelength) {
    qFatal("Error reading mp3-file %d %d",ftell(file), feof(file));
  }
  fclose(file);


  mad_stream Stream;
  mad_frame Frame;
  mad_synth Synth;
  // Transfer it to the mad stream-buffer:
  mad_stream_init(&Stream);
  mad_stream_buffer(&Stream, inputbuf, mp3filelength);
  // Init the buffer for the decoded samples:
  buffer.resize(1000000); // lets start with 10megs of samples.
  // Decode the whole file:
  mad_frame_init(&Frame);
  mad_synth_init(&Synth);
  qDebug("Decoding mp3 file");
  
  long curr_sample=0;
  while (Stream.error!=MAD_ERROR_BUFLEN) {
      if (mad_frame_decode(&Frame,&Stream)) {
	  if (MAD_RECOVERABLE(Stream.error))	{
	      qWarning("Recoverable frame level error (%s)",
		       mad_stream_errorstr(&Stream));
	      continue;
	  } else
	      if (Stream.error==MAD_ERROR_BUFLEN) {
		  break;
	      } else {
		  qWarning("Unrecoverable frame level error (%s).",
			   mad_stream_errorstr(&Stream));
		  break;
	      }
      }
      // Once decoded the frame is synthesized to PCM samples. 
      mad_synth_frame(&Synth,&Frame);
      // Check if we need to enlarge the buffer:
      if (curr_sample+2*Synth.pcm.length > buffer.size()) {
	  try {
	      buffer.resize(buffer.size() + 1000000); // add ten more megs
	      qDebug("decoded %li samples...", curr_sample);
	  }
	  catch (...) {
	      qDebug("Could not allocate more memory.");
	  }
      }
      // Transfer to buffer:
      for (int i=0;i<Synth.pcm.length;i++) {
	  SAMPLE Sample;
	  /* Left channel */
	  Sample=(SAMPLE)(Synth.pcm.samples[0][i]>>(MAD_F_FRACBITS-15));
	  buffer[curr_sample++] = Sample;
	  /* Right channel. If the decoded stream is monophonic then
	   * the right output channel is the same as the left one.
	   */
	  if(MAD_NCHANNELS(&Frame.header)==2)
	      Sample=(SAMPLE)(Synth.pcm.samples[0][i]>>(MAD_F_FRACBITS-15));
	  buffer[curr_sample++] = Sample;
      }
  }
  buffer.resize(curr_sample);
  qDebug("decoded %li samples.", curr_sample);
  position = 0;
  delete inputbuf;
 }

SoundSourceHeavymp3::~SoundSourceHeavymp3() {}

long SoundSourceHeavymp3::seek(long pos) {
    position = pos;
    return pos;
}

unsigned SoundSourceHeavymp3::read(unsigned long size, const SAMPLE* _destination) {
    SAMPLE *destination = (SAMPLE*) _destination;
    for (unsigned int i=0; i<size; i++)
        destination[i] = buffer[position++];
    return size;
}

long unsigned SoundSourceHeavymp3::length() {
    return buffer.size();
}
