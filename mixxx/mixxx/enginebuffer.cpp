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

EngineBuffer::EngineBuffer(DlgPlaycontrol *playcontrol, DlgChannel *channel, MidiObject *midi, char *filename)
{
  PlayButton = new ControlPushButton("playbutton", simulated_latching, PORT_B, 0, midi);
  PlayButton->setValue(on);
  connect(playcontrol->PushButtonPlay, SIGNAL(pressed()), PlayButton, SLOT(pressed()));
  connect(playcontrol->PushButtonPlay, SIGNAL(released()), PlayButton, SLOT(released()));
  connect(PlayButton, SIGNAL(valueChanged(valueType)), this, SLOT(slotUpdatePlay(valueType)));

  rateSlider = new ControlPotmeter("rateslider", ADC3, midi, 0.9,1.1);
  rateSlider->slotSetPosition(64);
  rate = rateSlider->getValue();
  connect(channel->SliderRate, SIGNAL(valueChanged(int)), rateSlider, SLOT(slotSetPosition(int)));
  connect(rateSlider, SIGNAL(valueChanged(FLOAT)), this, SLOT(slotUpdateRate(FLOAT)));
  /*
    Open the file:
  */
  int i=strlen(filename)-1;
  while ((filename[i] != '.') && (i>0))
    i--;
  if (i == 0) {
    qFatal("Wrong filename: %s.",filename);
    std::exit(-1);
  }
  char ending[80];
  strcpy(ending,&filename[i]);
  if (!strcmp(ending,".wav"))
    file = new AFlibfile(filename);
  else if (!strcmp(ending,".mp3") || (!strcmp(ending,".MP3")))
	file = new mp3file(filename);

  if (file==0) {
    qFatal("Error opening %s", filename);
    std::exit(-1);
  }
  // Allocate temporary buffer
  read_buffer_size = READBUFFERSIZE;
  chunk_size = READCHUNKSIZE;
  temp = new SAMPLE[2*chunk_size]; // Temporary buffer for the raw samples
  // note that the temp buffer is made extra large.
  readbuffer = new CSAMPLE[read_buffer_size];

  // Initialize position in read buffer:
  filepos = 0;
  frontpos = 0;
  play_pos = 0;
  direction = 1;

  // Allocate semaphore
  buffers_read_ahead = new sem_t;

  // ...and read one chunk to get started:
  getchunk();

}

EngineBuffer::~EngineBuffer(){
  delete [] temp;
  delete [] readbuffer;
  delete buffers_read_ahead;
  delete file;
  delete PlayButton;
}

void EngineBuffer::start()
{
	qDebug("starting EngineBuffer...");
	QThread::start();
	qDebug("started!");
}

void EngineBuffer::run()
{
  //pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,0);

	qDebug(".");
  while(true)
  {
//	qDebug(".");
    // Wait for playback if in buffer is filled.
    sem_wait(buffers_read_ahead);
    // Check if the semaphore is too large:
    int sem_value;
    sem_getvalue(buffers_read_ahead, &sem_value);
    if (sem_value != 0)
	;
    else
      // Read a new chunk:
      getchunk();
  }
};


void EngineBuffer::slotUpdatePlay(valueType newvalue) {
  qDebug("playbutton touched");
  slotUpdateRate(rateSlider->getValue());
}

void EngineBuffer::slotUpdateRate(FLOAT r)
{
	if (PlayButton->getValue()==on)
		rate = r;
	else
		rate = 0.;
	qDebug("Rate value: %f",rate);
}

void EngineBuffer::getchunk() {
  // Save direction if player changes it's mind while we're processing.
  //int saved_direction = direction;

  // update frontpos so that we wont be called while reading:
  frontpos = (frontpos+chunk_size)%read_buffer_size;
  qDebug("Reading...");
  //std::cout << "Starting reading chunk " << saved_direction << ".\n";
  // For a read backwards, we have to change the position in the file:
  /*if (direction == -1) {
    filepos -= (read_buffer_size + chunk_size);
    file->seek(filepos);
    //afSeekFrame(fh, AF_DEFAULT_TRACK, (AFframecount) (filepos/channels));
  } else*/
  // Read a chunk
  unsigned samples_read = file->read(chunk_size, temp);
  //samples_read = chunk_size;
  /*if (samples_read != chunk_size) {
     cout << "Read from file failed: " << samples_read << ":" << chunk_size  <<
 "\n" << flush;
    //exit(-1);
    }*/
  // Convert from SAMPLE to CSAMPLE. Should possibly be optimized
  // using assembler code from music-dsp archive.
  filepos += samples_read;
  unsigned new_frontpos =
 (frontpos-chunk_size+read_buffer_size)%read_buffer_size;
  for (unsigned j=0; j<samples_read; j++) {
    readbuffer[new_frontpos] = temp[j];
    new_frontpos ++;
    if (new_frontpos > read_buffer_size) new_frontpos = 0;
  }

  frontpos = new_frontpos;
  qDebug("Done reading.");
//  statuswin->print(2,20,"          ");
  //cout << "New filepos " << filepos << ":" << frontpos << "\n" << flush;
  // std::cout << "Finished read.\n" << flush;
}
/*
  Moves the playpos forward change%
*/
void EngineBuffer::seek(FLOAT change) {
  double new_play_pos = play_pos + change*file->length();
  if (new_play_pos > file->length()) new_play_pos = file->length();
  if (new_play_pos < 0) new_play_pos = 0;
  filepos = (long unsigned)new_play_pos;
  frontpos = 0;
  cout << change << " ";
  qDebug("Seeking...");
  //cout << "Seeking...\n";
  file->seek(filepos);
  getchunk();
  //cout << "done seeking.\n";
//  statuswin->print(2,1,"          ");
  play_pos = new_play_pos;

}

bool even(long n) {
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
void EngineBuffer::checkread() {
  static int sem_value; // place to store the value of the semaphore for read
  static int pending_time = 0;

  bool send_request = false;

  if ((distance((long)floor(play_pos)%read_buffer_size, frontpos)
      < READAHEAD*BUFFER_SIZE) && (filepos != (unsigned long) file->length())) {
    direction = 1;
    send_request = true;
  } else
    if ((distance(frontpos, (long)floor(play_pos)%read_buffer_size)
	< READAHEAD*BUFFER_SIZE) && (filepos > read_buffer_size)){
      direction = -1;
      send_request = true;
    }

  if (send_request) {
    // check if we still have a request pending:
    //std::cout << (long)play_pos << ", " << frontpos << ", " << filepos << "\n";
    sem_getvalue(buffers_read_ahead, &sem_value);
    if (sem_value == 0) {
	  //qDebug("Frontpos: %i Playpos: %i),frontpos,play_pos);
      //std::cout << frontpos << "," <<play_pos<<","<<(long)floor(play_pos)%read_buffer_size<<"\n";
      //cout << filepos << "," << filelength << "\n";
      sem_post(buffers_read_ahead);
      pending_time = 0;
    }
    else {
      pending_time ++;
      //std::cout << pending_time << "\n";
      if (pending_time == 0.9*READAHEAD)
	qDebug("Warning: reader is (close to) lacking behind player!");
    }
  }
}

/*
  Helper function which returns the distance in the readbuffer between
  _start and end.
*/
long EngineBuffer::distance(const long _start, const long end) {
  long start = _start;
  if (start > end)
    start -= read_buffer_size;
  return end-start;
}

void EngineBuffer::writepos() {
  static FLOAT lastwrite = 0.;
  // Write position to screen:
  FLOAT newwrite = play_pos/file->length();
  //cout << newwrite << ",";
  if (floor(fabs(newwrite-lastwrite)*100) >= 1) {
    char str[10];
    sprintf(str, "%6.1f%%", 100*newwrite);
    //qDebug("%s",str);
    lastwrite = newwrite;
  }
}

 FLOAT EngineBuffer::min(const FLOAT a, const FLOAT b) {
  if (a > b)
    return b;
  else
    return a;
}

FLOAT EngineBuffer::max(const FLOAT a, const FLOAT b) {
  if (a > b)
    return a;
  else
    return b;
}

void EngineBuffer::process(CSAMPLE *, CSAMPLE *buffer, int buf_size) {
  long prev;

  for (int i=0; i<buf_size; i+=2) {
    prev = (long)floor(play_pos)%read_buffer_size;
    if (!even(prev)) prev--;
    long next = (prev+2)%read_buffer_size;
    FLOAT frac = play_pos - floor(play_pos);
    buffer[i ] = readbuffer[prev  ] +frac*(readbuffer[next  ]-readbuffer[prev  ]);
    buffer[i+1] = readbuffer[prev+1] +frac*(readbuffer[next+1]-readbuffer[prev+1]);
    play_pos += 2.*rate;
    play_pos = max(0.,min(file->length(), play_pos));
  }

  checkread();
  writepos();
  // Check the wheel:
  //rate->checkwheelpos(BUFFER_SIZE);
}
