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
#include "wplaybutton.h"
#include "wwheel.h"
#include "wknob.h"
#include <qslider.h>
#include "wslider.h"
#include "wplayposslider.h"
#include "configobject.h"
#include <qstring.h>
#include <qlcdnumber.h>
#include <qevent.h>
#include <qfileinfo.h>
#include <qevent.h>
#include "soundsourcemp3.h"
#ifdef __UNIX__
  #include "soundsourceaudiofile.h"
#endif
#ifdef __WIN__
  #include "soundsourcesndfile.h"
#endif

EngineBuffer::EngineBuffer(QApplication *a, QWidget *m, DlgPlaycontrol *_playcontrol, const char *_group, const char *filename)
{
  app = a;
  mixxx = m;
  visualBuffer = 0;
  group = _group;
  
  playposSliderLast = 0.;
  playcontrol = _playcontrol;
  start_seek = -1;

  // Play button
  PlayButton = new ControlPushButton(ConfigKey(group, "play"), simulated_latching);
  playcontrol->PushButtonPlay->controlButton = PlayButton;
  connect(playcontrol->PushButtonPlay, SIGNAL(pressed()), PlayButton, SLOT(pressed()));
  connect(playcontrol->PushButtonPlay, SIGNAL(released()), PlayButton, SLOT(released()));
  connect(PlayButton, SIGNAL(valueChanged(valueType)), this, SLOT(slotUpdatePlay(valueType)));

  // Playback rate slider
  rateSlider = new ControlPotmeter(ConfigKey(group, "rate"), 0.9, 1.1);
  rateSlider->slotSetPosition(64);
  rate.write(rateSlider->getValue());
  connect(playcontrol->SliderRate, SIGNAL(valueChanged(int)), rateSlider, SLOT(slotSetPosition(int)));
  connect(rateSlider, SIGNAL(valueChanged(FLOAT_TYPE)), this, SLOT(slotUpdateRate(FLOAT_TYPE)));
  connect(rateSlider, SIGNAL(updateGUI(int)), playcontrol->SliderRate, SLOT(setValue(int)));

  // Wheel to control playback position/speed
  wheel = new ControlRotary(ConfigKey(group, "wheel"), PlayButton);
  connect(playcontrol->WheelPlaycontrol, SIGNAL(valueChanged(int)), wheel, SLOT(slotSetValue(int)));
  connect(wheel, SIGNAL(valueChanged(FLOAT_TYPE)), this, SLOT(slotUpdateRate(FLOAT_TYPE)));
  // Don't connect this, it results in an infinite loop:
//  connect(wheel, SIGNAL(updateGUI(int)), playcontrol->WheelPlaycontrol, SLOT(setValue(int)));
  connect(wheel, SIGNAL(updateGUI(int)), wheel, SLOT(slotSetPosition(int)));

  // Slider to show and change song position
  connect(playcontrol->SliderPosition, SIGNAL(valueChanged(int)), this, SLOT(slotPosition(int)));

  // Allocate temporary buffer
  temp = new SAMPLE[READCHUNKSIZE*2]; // Temporary buffer for the raw samples
  read_buffer = new CSAMPLE[READBUFFERSIZE];
  for (unsigned i=0; i<READBUFFERSIZE; ++i)
      read_buffer[i] = 0.;

  // Allocate semaphore
  buffersReadAhead = new QWaitCondition();

  // Semaphore for stopping thread
  requestStop = new QSemaphore(1);

  // Semaphore controlling access to getchunk
  readChunkLock = new QSemaphore(1);

  // Open the track:
  file = 0;
  newtrack(filename);
  PlayButton->setValue(off); // stop the first track from playing.

  // Allocate buffer for processing:
  buffer = new CSAMPLE[MAX_BUFFER_LEN];
}

EngineBuffer::~EngineBuffer()
{
    if (running())
        stop();

    if (file != 0)
        delete file;
    delete [] temp;
    delete [] read_buffer;

    delete buffersReadAhead;
    delete requestStop;
    delete readChunkLock;

    delete PlayButton;
    delete wheel;
    delete rateSlider;
    delete buffer;
}

const char *EngineBuffer::getGroup()
{
    return group;
}

void EngineBuffer::setVisual(QObject *_visualBuffer)
{
    visualBuffer = _visualBuffer;
}

void EngineBuffer::newtrack(const char* filename)
{
    // Start track in pause state
    pause = true;

    // If we are already playing a file, then get rid of it:
    if (file != 0)
    {
        delete file;
        file = 0;
    }

    if (filename != 0)
    {
        // Check if filename is valid
        QFileInfo finfo(filename);
        if (finfo.exists())
        {
            if (finfo.extension(false).upper() == "WAV")
#ifdef __UNIX__
                file = new SoundSourceAudioFile(filename);
#endif
#ifdef __WIN__
                file = new SoundSourceSndFile(filename);
#endif
            else if (finfo.extension(false).upper() == "MP3")
                file = new SoundSourceMp3(filename);
        }
    } else
#ifdef __UNIX__
        file = new SoundSourceAudioFile("/dev/null");
#endif
#ifdef __WIN__
        file = new SoundSourceSndFile("/dev/null");
#endif  

    if (file==0)
        qFatal("Error opening %s", filename);

    // Initialize position in read buffer:
    filepos_start.write(0.);
    filepos_end.write(0.);
    filepos_play.write(0.);
    bufferpos_start = 0.;
    bufferpos_end = 0.;
    bufferpos_play.write(0.);

    visualPlaypos = 0;
    visualRate = 0.;

    // ...and read one chunk to get started:
    getchunk();

    if (file != 0)
    {
        /*
        Write to playcontrol:
        */
        QString title = filename;
        // Prune path from filename:
        title = title.section('/',-1);
        //title = title.section('\\',-1);
        // Prune last ending from filename:
        title = title.section('.',0,-2);
        // Finish the title string:
        title = QString("Title : ") + title + "\n\n";

        int seconds = file->length()/(2*SRATE);
        QString tmp;
        tmp.sprintf("Length : %02d:%02d\n\n", seconds/60, seconds - 60*(seconds/60));
        title += tmp;
        title += "Type  : " + file->type;

        playcontrol->textLabelTrack->setText(title);
        pause = false;
    }
}

void EngineBuffer::stop()
{
    buffersReadAhead->wakeAll();

    (*requestStop)++;
    wait();
    (*requestStop)--;
}

void EngineBuffer::run()
{
    rtThread();

    while(requestStop->available())
    {
        // Wait for playback if in buffer is filled.
        buffersReadAhead->wait();

        // Read a new chunk:
        getchunk();
    }
};
/*
  Called when the playbutten is pressed
*/
void EngineBuffer::slotUpdatePlay(valueType)
{
    if (PlayButton->getPosition()==down)
    {
        //qDebug("Entered seeking mode");
        rate.write(0);
        start_seek = wheel->getPosition();
    }
    else if (PlayButton->getPosition()==up && start_seek>=0)
    {
        int end_seek = wheel->getPosition();
        if (abs(start_seek - end_seek) > 2)
        {
            // A seek has occured. Find new filepos:
            if ((wheel->direction==1) && (end_seek < start_seek))
                end_seek += 128;
            else
                if ((wheel->direction==-1) && (end_seek > start_seek))
                    end_seek -= 128;

            seek((FLOAT_TYPE)(end_seek-start_seek)/128);
        }
        start_seek = -1;
        //qDebug("Ended seeking");
    }
    slotUpdateRate(rateSlider->getValue());
}

/*
  Called when the wheel is turned or the rate slider is moved:
*/
void EngineBuffer::slotUpdateRate(FLOAT_TYPE)
{
    //qDebug("1: Rate value: %f, wheel value: %f",rate.read(),wheel->getValue());


    // If playbutton is on rateslider value is used as basic rate added with the wheel value
    if (PlayButton->getValue()==on)
    {
        rate.write(rateSlider->getValue() + 4.*wheel->getValue());
    }
    // If the playbutton is off, but pressed down, seeking is taking place, and rate is set to 0
    else if (PlayButton->getPosition()==down)
    {
        // No rate while seeking:
        rate.write(0.);
    }
    // If playbutton is off, and not pressed down, use the wheel to get the rate
    else
        rate.write(3.*wheel->getValue());
    
    //qDebug("Rate value: %f, wheel value: %f",rate.read(),wheel->getValue());
    
}

/*
  Read a new chunk into the readbuffer:
*/
void EngineBuffer::getchunk()
{
    //qDebug("Reading..., playpos %f",playpos_buffer.read());

    (*readChunkLock)++;

    // Determine playback direction
    bool backwards;
    if (rate.read() < 0.)
        backwards = true;
    else
        backwards = false;

    // Determine new start and end positions in file and buffer, start index of where read samples
    // will be placed in read buffer (bufIdx), and perform seek if reading backwards
    double filepos_start_new, filepos_end_new;
    int bufIdx;
    if (backwards)
    {
        filepos_start_new = max(0.,filepos_start.read()-(double)READCHUNKSIZE);    
        bufferpos_start = (bufferpos_start-READCHUNKSIZE+READBUFFERSIZE)%READBUFFERSIZE;
        file->seek(filepos_start_new);

        if ((filepos_end.read()-filepos_start_new)/READCHUNKSIZE < READCHUNK_NO)
            filepos_end_new = filepos_end.read();
        else
        {
            filepos_end_new = filepos_end.read()-(double)READCHUNKSIZE;
            bufferpos_end   = bufferpos_start; //(bufferpos_end-READCHUNKSIZE+READBUFFERSIZE)%READBUFFERSIZE;
        }
        
        bufIdx = bufferpos_start;
    }
    else
    {
        filepos_end_new = filepos_end.read()+(double)READCHUNKSIZE;
        bufIdx = bufferpos_end;
        bufferpos_end   = (bufferpos_end+READCHUNKSIZE)%READBUFFERSIZE;
        if ((filepos_end.read()-filepos_start.read())/READCHUNKSIZE < READCHUNK_NO)
            filepos_start_new = filepos_start.read();
        else
        {
            filepos_start_new = filepos_start.read()+READCHUNKSIZE;
            bufferpos_start = bufferpos_end; //(bufferpos_start+READCHUNKSIZE)%READBUFFERSIZE;
        }
    }
    filepos_start.write(filepos_start_new);
    filepos_end.write(filepos_end_new);
                                                
    // Read samples
    file->read(READCHUNKSIZE, temp);

    // Seek to end of the samples read in buffer, if we are reading backwards. This is to ensure, that the correct samples
    // are read, if we next time are going forward.
    if (backwards)
        file->seek(filepos_end.read());
    
    // Copy samples to read_buffer
    int i=0;
    for (int j=bufIdx; j<bufIdx+READCHUNKSIZE; j++)
        read_buffer[j] = (CSAMPLE)temp[i++];
        
    // Update variables used to copy the buffer to a vertex buffer for 3D visualization
    visualPos1 = bufIdx;
    visualLen1 = READCHUNKSIZE;
    visualPos2 = 0;
    visualLen2 = 0;
        
    (*readChunkLock)--;

    // Send user event to main thread, indicating that the visual sample buffer should be updated
    if (visualBuffer>0)
        postEvent(visualBuffer,new QEvent((QEvent::Type)1001));
}

/*
  This is called when the positionslider is released:
*/
void EngineBuffer::slotPosition(int newvalue)
{
    seek((FLOAT_TYPE)newvalue/102. - filepos_play.read()/(FLOAT_TYPE)file->length());
}

/*
  Moves the playpos forward change%
*/
void EngineBuffer::seek(FLOAT_TYPE change)
{
qDebug("seeking...");
    (*readChunkLock)++;

    pause = true;

    double new_playpos = filepos_play.read() + change*file->length();
    if (new_playpos > file->length())
        new_playpos = file->length();
    if (new_playpos < 0)
        new_playpos = 0;

    filepos_play.write(new_playpos);
    filepos_start.write(new_playpos);
    filepos_end.write(new_playpos);

    bufferpos_play.write(0.);
    bufferpos_start = 0.;
    bufferpos_end = 0.;

    for (int i=0; i<READBUFFERSIZE; i++)
        read_buffer[i] = 0.;

    //qDebug("Seeking %g to %g",change, playpos_file.read());
    file->seek((long unsigned)filepos_play.read());

    (*readChunkLock)--;

    buffersReadAhead->wakeAll();
    pause = false;

    visualPlaypos = (int)bufferpos_play.read()%READBUFFERSIZE;
}

inline bool even(long n)
{
//    if ((n/2) != (n+1)/2)
    if (n%2 != 0)
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
    if (rate.read()>=0. /*&& filepos_end.read()< file->length()*/ && (filepos_end.read() - filepos_play.read() < READCHUNKSIZE*(READCHUNK_NO/2-2)))
    {
        //qDebug("forw: diff %f,%f",filepos_end.read(),filepos_play.read());
        buffersReadAhead->wakeAll();
    }
    else if (rate.read()<0. /*&& filepos_start.read()>0.*/ && (filepos_play.read() - filepos_start.read() < READCHUNKSIZE*(READCHUNK_NO/2-2))) 
    {
        //qDebug("back: diff %f,%f",filepos_play.read(),filepos_start.read());
        buffersReadAhead->wakeAll();
    }
}

// Write the position in the buffer on the LCD display.
// Called from process.
void EngineBuffer::writepos()
{
    playposSliderNew = (filepos_play.read()/file->length())*100.;
    if (floor(fabs(playposSliderNew-playposSliderLast)) >= 1.)
    {
        // Send User event
        postEvent(mixxx,new QEvent(QEvent::User));

        // Store old position
        playposSliderLast = playposSliderNew;
    }
}

CSAMPLE *EngineBuffer::process(const CSAMPLE *, const int buf_size)
{
    if (rate.read()==0. || pause)
    {
        for (int i=0; i<buf_size; i++)
            buffer[i]=0.;
        return buffer;
    }
    else
    {
        // Get rate and playpos
        double myRate=rate.read()*BASERATE;

        // Check if we are at the boundaries of the file
        if (filepos_play.read()<0. || filepos_play.read()>file->length())
        {
            if (filepos_play.read()<0.)
                filepos_play.write(0.);
            else
                filepos_play.write(file->length());

            for (int i=0; i<buf_size; i++)
                buffer[i] = 0.;
            return buffer;
        }

        // Determine playback direction
        bool backwards = false;
        if (myRate<0.)
            backwards = true;

        //double abs_rate = fabs(myRate);
        double rate_add = 2.*myRate;

        // Determine position in read_buffer to start from (idx)
        double idx = bufferpos_play.read();

        // Prepare buffer
        if (backwards)
        {
            for (int i=0; i<buf_size; i+=2)
            {
                long prev = (long)(floor(idx)+READBUFFERSIZE)%READBUFFERSIZE;
                if (!even(prev)) prev--;
                long next = (prev-2+READBUFFERSIZE)%READBUFFERSIZE;
                
                FLOAT_TYPE frac = idx-floor(idx);
                buffer[i  ] = read_buffer[prev  ] + frac*(read_buffer[next  ]-read_buffer[prev  ]);
                buffer[i+1] = read_buffer[prev+1] + frac*(read_buffer[next+1]-read_buffer[prev+1]);

                idx += rate_add;
            }
        }
        else
        {
            for (int i=0; i<buf_size; i+=2)
            {
                long prev = (long)floor(idx)%READBUFFERSIZE;
                if (!even(prev)) prev--;

                long next = (prev+2)%READBUFFERSIZE;

                FLOAT_TYPE frac = idx - floor(idx);
                buffer[i  ] = read_buffer[prev  ] + frac*(read_buffer[next  ]-read_buffer[prev  ]);
                buffer[i+1] = read_buffer[prev+1] + frac*(read_buffer[next+1]-read_buffer[prev+1]);

                idx += rate_add;
            }
        }
        
        // Write buffer playpos (ensure valid range)
        if (idx>READBUFFERSIZE)
            idx -= (double)READBUFFERSIZE;
        else if (idx<0)
            idx += (double)READBUFFERSIZE;
        bufferpos_play.write(idx);
        
        // Write file playpos
        filepos_play.add(myRate*(double)buf_size);
        
        // Update visual rate and playpos
        visualPlaypos = idx;
        visualRate = myRate;
    }

    if (!pause)
    {
        checkread();
        // Check the wheel:
        wheel->updatecounter(buf_size,EngineObject::SRATE);
        // Write position to the gui:
        writepos();
    }

    return buffer;
}
