/***************************************************************************
                          enginebuffer.cpp  -  description
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

#include "enginebuffer.h"
#include <qlabel.h>

EngineBuffer::EngineBuffer(DlgPlaycontrol *_playcontrol, int midiPlaybutton, int midiRateslider,
                           int midiWheel, MidiObject *midi, const char *filename)
{
  playcontrol = _playcontrol;

  PlayButton = new ControlPushButton("playbutton", simulated_latching, PORT_B, midiPlaybutton, midi);
  PlayButton->setValue(on);
  connect(playcontrol->PushButtonPlay, SIGNAL(pressed()), PlayButton, SLOT(pressed()));
  connect(playcontrol->PushButtonPlay, SIGNAL(released()), PlayButton, SLOT(released()));
  connect(PlayButton, SIGNAL(valueChanged(valueType)), this, SLOT(slotUpdatePlay(valueType)));

  rateSlider = new ControlPotmeter("rateslider", midiRateslider, midi, 0.9,1.1);
  rateSlider->slotSetPosition(64);
  rate.write(rateSlider->getValue());
  connect(playcontrol->SliderRate, SIGNAL(valueChanged(int)), rateSlider, SLOT(slotSetPosition(int)));
  connect(rateSlider, SIGNAL(valueChanged(FLOAT_TYPE)), this, SLOT(slotUpdateRate(FLOAT_TYPE)));
  connect(rateSlider, SIGNAL(recievedMidi(int)), playcontrol->SliderRate, SLOT(setValue(int)));

  wheel = new ControlRotary("wheel", midiWheel, midi);
  connect(playcontrol->SliderPlaycontrol, SIGNAL(valueChanged(int)), wheel, SLOT(slotSetPosition(int)));
  connect(wheel, SIGNAL(valueChanged(FLOAT_TYPE)), this, SLOT(slotUpdateRate(FLOAT_TYPE)));
  connect(wheel, SIGNAL(recievedMidi(int)), playcontrol->SliderPlaycontrol, SLOT(setValue(int)));

  connect(this, SIGNAL(position(int)), playcontrol->LCDposition, SLOT(display(int)));

  //connect(this, SIGNAL(position(int)), playcontrol->SliderPosition, SLOT(setValue(int)));
  connect(playcontrol->SliderPosition, SIGNAL(valueChanged(int)), this, SLOT(slotPosition(int)));

  // Allocate temporary buffer
  read_buffer_size = READBUFFERSIZE;
  chunk_size = READCHUNKSIZE;
  temp = new SAMPLE[3*chunk_size]; // Temporary buffer for the raw samples
  read_buffer = new CSAMPLE[read_buffer_size];

  // Allocate semaphore
  buffersReadAhead = new QWaitCondition();

  // Semaphore for stopping thread
  requestStop = new QSemaphore(1);

  // Open the track:
  file = 0;
  newtrack(filename);

  // Allocate buffer for processing:
  buffer = new CSAMPLE[MAX_BUFFER_LEN];
}

EngineBuffer::~EngineBuffer(){
  qDebug("EngineBuffer: dealloc buffer");
  if (running())
  {
    qDebug("Stopping buffer");
    stop();
  }
  qDebug("buffer waiting...");

  qDebug("buffer actual dealloc");
  if (file != 0) delete file;
  delete [] temp;
  delete [] read_buffer;

  delete buffersReadAhead;
  delete requestStop;

  delete PlayButton;
  delete wheel;
  delete rateSlider;
  delete buffer;
}

void EngineBuffer::newtrack(const char* filename) {
  // If we are already playing a file, then get rid of it:
  if (file != 0) delete file;
  /*
    Open the file:
  */
  int i=strlen(filename)-1;
  while ((filename[i] != '.') && (i>0))
    i--;
  if (i == 0) {
    qFatal("Wrong filename: %s.",filename);
  }
  char ending[80];
  strcpy(ending,&filename[i]);
#ifndef Q_WS_WIN
  if (!strcmp(ending,".wav"))
    file = new SoundSourceAFlibfile(filename);
  else 
#endif
  if (!strcmp(ending,".mp3") || (!strcmp(ending,".MP3")))
	file = new SoundSourceMp3(filename);

  if (file==0) {
    qFatal("Error opening %s", filename);
  }
  // Write to playcontrol:
  playcontrol->textLabelTrack->setText(filename);

  // Initialize position in read buffer:
  lastread_file.write(0.);
  playpos_file.write(0.);
  playpos_buffer.write(0.);
  // ...and read one chunk to get started:
  getchunk();
}

void EngineBuffer::start() {
    qDebug("starting EngineBuffer...");
    QThread::start();
    qDebug("started!");
}

void EngineBuffer::stop()
{
  buffersReadAhead->wakeAll();

  requestStop->operator++(1);
  wait();
  requestStop->operator--(1);
}

void EngineBuffer::run() {
  while(requestStop->available()) {
    // Wait for playback if in buffer is filled.
    buffersReadAhead->wait();

    // Read a new chunk:
    getchunk();
  }
};
/*
  Called when the playbutten is pressed
*/
void EngineBuffer::slotUpdatePlay(valueType) {
    if (PlayButton->getPosition()==down) {
	qDebug("Entered seeking mode");
	rate.write(0);
	start_seek = wheel->getPosition();
    }
    else if (PlayButton->getPosition()==up) {
	seek((FLOAT_TYPE)(end_seek()-start_seek)/128);
	qDebug("Ended seeking");
    }
    slotUpdateRate(rateSlider->getValue());
}

int EngineBuffer::end_seek() {
    int _end_seek = wheel->getPosition();
    if (abs(start_seek - _end_seek) > 2) {
	// A seek has occured. Find new filepos:
	if ((wheel->direction==1) &&(_end_seek < start_seek))
	    _end_seek += 128;
	else
	    if ((wheel->direction==-1) && (_end_seek > start_seek))
		_end_seek -= 128;
    } 
    return _end_seek;
}
/*
  Called when the wheel is turned or the rate slider is moved:
*/
void EngineBuffer::slotUpdateRate(FLOAT_TYPE)
{
    if (PlayButton->getValue()==on)
        rate.write(rateSlider->getValue() + 4*wheel->getValue());
    else
        if (PlayButton->getPosition()==down) {
	    // No rate while seeking:
            rate.write(0);
	    emit position((int)(100*(FLOAT_TYPE)(end_seek()-start_seek)/128));
	}
        else
            rate.write(4*wheel->getValue());

    //qDebug("Rate value: %f",rate.read());
}

/*
  Read a new chunk into the readbuffer:
*/
void EngineBuffer::getchunk() {
    if (readChunkLock.read()==0.) {
	readChunkLock.write(1.);

  qDebug("Reading...");

  // Read a chunk
  unsigned samples_read = file->read(chunk_size, temp);

qDebug("1");

  if (samples_read < chunk_size)
      qDebug("Didn't get as many samples as we asked for: %d:%d", chunk_size, samples_read);

qDebug("2");
  // Convert from SAMPLE to CSAMPLE. Should possibly be optimized
  // using assembler code from music-dsp archive.
  unsigned long lastread_buffer = ((unsigned long)(playpos_buffer.read() + lastread_file.read() -
         playpos_file.read()))%read_buffer_size;
qDebug("3");

   /* qDebug("lastread_buffer: %f", (double)lastread_buffer);
    qDebug("playpos_buffer: %f",playpos_buffer.read());
    qDebug("playpos_file: %f",playpos_file.read());
    qDebug("lastread_file: %f",lastread_file.read()); */

  unsigned i = 0;
  qDebug("p: %p,%p, j=%i to %i",read_buffer,temp,lastread_buffer,min(read_buffer_size,lastread_buffer+samples_read));
  for (unsigned long j=lastread_buffer; j<min(read_buffer_size,lastread_buffer+samples_read); j++)
    read_buffer[j] = temp[i++];
qDebug("4");
  //qDebug("%i",lastread_buffer+samples_read-read_buffer_size);
  for (signed long j=0; j<(signed long)(lastread_buffer+samples_read-read_buffer_size); j++)
    read_buffer[j] = temp[i++];
qDebug("5");

  // Update lastread_file position:
  lastread_file.add((double)samples_read);
  qDebug("Done reading.");
    
    readChunkLock.write(0.);
    } else
	qDebug("getchunk not processed");

}

/*
  This is called when the positionslider is released:
*/
void EngineBuffer::slotPosition(int newvalue) {
  seek((FLOAT_TYPE)newvalue/102 - playpos_file.read()/(FLOAT_TYPE)file->length());
}
/*
  Moves the playpos forward change%
*/
void EngineBuffer::seek(FLOAT_TYPE change)
{
    if (readChunkLock.read()==0.) {
        readChunkLock.write(1.);

    qDebug("Entered seek");
  double saved_rate = rate.read();
  rate.write(0);
  qDebug("Set rate to zero");
  double new_playpos = playpos_file.read() + change*file->length();
  if (new_playpos > file->length()) new_playpos = file->length();
  if (new_playpos < 0) new_playpos = 0;
  playpos_file.write(new_playpos);
  playpos_buffer.write(0.);
  lastread_file.write(new_playpos);
  qDebug("Seeking %g to %g",change, playpos_file.read());
  file->seek((long unsigned)playpos_file.read());
  //getchunk();
  buffersReadAhead->wakeAll();
  qDebug("done seeking.");
  rate.write(saved_rate);
  readChunkLock.write(0.);
} else {
	qDebug("no seeking since getChunk is active!");
}
}

bool even(long n)
{
    if ((n/2) != (n+1)/2)
        return false;
    else
        return true;
}

// -------- ------------------------------------------------------
// Purpose: Make a check if it is time to start reading some
//          more samples. If it is, update the semaphore.
// Input:   -
// Output:  -
// -------- ------------------------------------------------------
void EngineBuffer::checkread()
{
  if ((lastread_file.read() - playpos_file.read()) < READAHEAD)
    buffersReadAhead->wakeAll();
  else {
      if (lastread_file.read() - playpos_file.read() < 0.1*READAHEAD)
    	qDebug("Warning: reader is (close to) lacking behind player!");
  }
}


void EngineBuffer::writepos()
{
  static FLOAT_TYPE lastwrite = 0.;
  FLOAT_TYPE newwrite = playpos_file.read()/file->length();
  if (floor(fabs(newwrite-lastwrite)*100) >= 1) {
      emit position((int)(100*newwrite));
      lastwrite = newwrite;
  }
}

CSAMPLE *EngineBuffer::process(const CSAMPLE *, const int buf_size)
{
    if (rate.read()==0.)
    {
	    for (int i=0; i<buf_size; i++)
	        buffer[i]=0.;
    } else {
        long prev;
	FLOAT_TYPE myRate=rate.read();
	FLOAT_TYPE myPlaypos_buffer = playpos_buffer.read();
        FLOAT_TYPE myPlaypos_file = playpos_file.read();

	for (int i=0; i<buf_size; i+=2)
        {
            if (myPlaypos_file < file->length())
            {
  	            prev = (long)floor(myPlaypos_buffer)%read_buffer_size;
	            if (!even(prev)) prev--;
                long next = (prev+2)%read_buffer_size;
	            FLOAT_TYPE frac = myPlaypos_buffer - floor(myPlaypos_buffer);
                buffer[i  ] = read_buffer[prev  ] +frac*(read_buffer[next  ]-read_buffer[prev  ]);
	            buffer[i+1] = read_buffer[prev+1] +frac*(read_buffer[next+1]-read_buffer[prev+1]);
                double rate_add = 2*myRate;
	            myPlaypos_buffer +=rate_add;
                myPlaypos_file += rate_add;
            } else {
                buffer[i  ] = 0.;
	            buffer[i+1] = 0.;
            }
        }

	playpos_buffer.write(myPlaypos_buffer);
	playpos_file.write(myPlaypos_file);

        checkread();

        // Check the wheel:
        wheel->updatecounter(buf_size,EngineObject::SRATE);

        // Write position to the gui:
        writepos();
    }

    return buffer;
}
