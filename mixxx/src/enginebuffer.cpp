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
#include <qslider.h>
#include <qstring.h>
#include <qlcdnumber.h>
#include <qevent.h>
#include <qfileinfo.h>
#include <qevent.h>
#include "wplaybutton.h"
#include "wwheel.h"
#include "wknob.h"
#include "wslider.h"
#include "wplayposslider.h"
#include "configobject.h"
#include "soundsourcemp3.h"
#include "rtthread.h"
#include "soundbuffer.h"
#include "mixxxvisual.h"
#include "visual/guichannel.h"
#include "visual/signalvertexbuffer.h"
#include "mixxx.h"
#include "controlpushbutton.h"
#include "controlpotmeter.h"
#include "controlttrotary.h"
#include "controlengine.h"


#ifdef __UNIX__
  #include "soundsourceaudiofile.h"
#endif
#ifdef __WIN__
  #include "soundsourcesndfile.h"
#endif

EngineBuffer::EngineBuffer(MixxxApp *_mixxx, DlgPlaycontrol *_playcontrol, const char *_group, const char *filename)
{
    mixxx = _mixxx;
    group = _group;
    playcontrol = _playcontrol;
    start_seek = -1;
    BASERATE = 1.;

    // Play button
    ControlPushButton *p = new ControlPushButton(ConfigKey(group, "play"));
    p->setWidget(playcontrol->PushButtonPlay);
    playButton = new ControlEngine(p);
    playButton->set(0);

    // Playback rate slider
    ControlPotmeter *p2 = new ControlPotmeter(ConfigKey(group, "rate"), 0.9, 1.1);
    p2->setWidget(playcontrol->SliderRate);
    rateSlider = new ControlEngine(p2);

    // Wheel to control playback position/speed
    ControlTTRotary *p3 = new ControlTTRotary(ConfigKey(group, "wheel"));
    p3->setWidget(playcontrol->WheelPlaycontrol);
    wheel = new ControlEngine(p3);

    // Slider to show and change song position
    p2 = new ControlPotmeter(ConfigKey(group, "playposition"), 0., 1.);
    p2->setWidget(playcontrol->SliderPosition);
    playposSlider = new ControlEngine(p2);
    playposSlider->setNotify(this);
    
    // Allocate sound buffer
    soundbuffer = new SoundBuffer(READCHUNKSIZE, READCHUNK_NO, WINDOWSIZE, STEPSIZE);

    // Semaphore for stopping thread
    requestStop = new QSemaphore(1);

    // Allocate semaphore
    buffersReadAhead = new QWaitCondition();

    // Semaphore controlling access to getchunk
    readChunkLock = new QSemaphore(1);

    // Open the track:
    file = 0;
    newtrack(filename);

    // If visual subsystem is present...
    guichannel = 0;
    if (mixxx->getVisual())
    {
        // Add buffer as a visual channel
        guichannel = mixxx->getVisual()->add(this);

        // Add soundbuffer as a visual signal to the channel
        guichannel->add(soundbuffer);
    }
    
    read_buffer_prt = soundbuffer->getChunkPtr(0);
                              
    // Allocate buffer for processing:
    buffer = new CSAMPLE[MAX_BUFFER_LEN];
}

EngineBuffer::~EngineBuffer()
{
    if (running())
        stop();

    delete soundbuffer;

    if (file != 0)
        delete file;

    delete playButton;
    delete wheel;
    delete rateSlider;
    delete buffer;
    delete buffersReadAhead;
    delete requestStop;
    delete readChunkLock;
}

const char *EngineBuffer::getGroup()
{
    return group;
}

SoundBuffer *EngineBuffer::getSoundBuffer()
{
    return soundbuffer;
}

int EngineBuffer::getPlaypos(int Srate)
{
    return (int)((CSAMPLE)visualPlaypos/(2.*(CSAMPLE)file->getSrate()/(CSAMPLE)Srate));
}


void EngineBuffer::newtrack(const char* filename)
{
    // Set pause while loading new track
    pause.lock();

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

    // Set base rate
    BASERATE = file->getSrate()/(FLOAT_TYPE)getPlaySrate();
        
    visualPlaypos = 0;
    visualRate = 0.;
    rate_exchange.write(BASERATE);
    
    soundbuffer->setSoundSource(file);

    // ...and read one chunk to get started:
    (*readChunkLock)++;
    soundbuffer->getchunk(BASERATE);
    (*readChunkLock)--;

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

        int seconds = file->length()/(2*file->getSrate());
        QString tmp;
        tmp.sprintf("Length : %02d:%02d\n\n", seconds/60, seconds - 60*(seconds/60));
        title += tmp;
        title += "Type  : " + file->type;

        playcontrol->textLabelTrack->setText(title);
    }

    // Reset playpos
    filepos_play = 0.;
    filepos_play_exchange.write(0.);
    bufferpos_play = 0.;
    playposSlider->set(0.);

    // Stop pausing process method
    pause.unlock();
}

void EngineBuffer::run()
{
    qDebug("enginebuffer running...");
    rtThread();

    while(requestStop->available())
    {
        // Wait for playback if in buffer is filled.
        buffersReadAhead->wait();

        (*readChunkLock)++;

        // Read a new chunk:
        soundbuffer->getchunk(rate_exchange.read());

        // Write playpos slider
        playposSlider->set(filepos_play_exchange.read()/file->length());

        // Send user event to main thread, indicating that the visual sample buffers should be updated
        if (guichannel>0)
            QThread::postEvent(guichannel,new QEvent((QEvent::Type)1001));

        (*readChunkLock)--;
    }
}

void EngineBuffer::stop()
{
    buffersReadAhead->wakeAll();

    (*requestStop)++;
    wait();
    (*requestStop)--;
}

void EngineBuffer::notify(double change)
{
    seek(change);
}

/*
  Moves the playpos to a new playpos
*/
void EngineBuffer::seek(FLOAT_TYPE change)
{
    //qDebug("seeking... %f",change);

    (*readChunkLock)++;
    
    pause.lock();

    double new_playpos = /*filepos_play + */change*file->length();
    if (new_playpos > file->length())
        new_playpos = file->length();
    if (new_playpos < 0)
        new_playpos = 0;

    soundbuffer->reset(new_playpos);
        
    filepos_play = new_playpos;
    filepos_play_exchange.write(filepos_play);
    
    bufferpos_play =0.;

    file->seek((long unsigned)filepos_play);

    (*readChunkLock)--;

    buffersReadAhead->wakeAll();
    
    pause.unlock();

    visualPlaypos = (int)bufferpos_play%READBUFFERSIZE;
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
void EngineBuffer::checkread(bool backwards)
{
    FLOAT_TYPE start = soundbuffer->getFileposStart();
    FLOAT_TYPE end = soundbuffer->getFileposEnd();
    
    if (!backwards && end< file->length() && (end - filepos_play < READCHUNKSIZE*(READCHUNK_NO/2-2)))
    {
        //qDebug("forw: diff %f,%f",filepos_end.read(),filepos_play.read());
        buffersReadAhead->wakeAll();
    }
    else if (backwards && start>0. && (filepos_play - start < READCHUNKSIZE*(READCHUNK_NO/2-2))) 
    {
        //qDebug("back: diff %f,%f",filepos_play.read(),filepos_start.read());
        buffersReadAhead->wakeAll();
    }
}

CSAMPLE *EngineBuffer::process(const CSAMPLE *, const int buf_size)
{
    if (pause.tryLock())
    {
        // Calculate rate
        double rate;
        if (playButton->get()==1.)
            rate=wheel->get()+rateSlider->get()*BASERATE;
        else
            rate=wheel->get()*BASERATE*20.;
        rate_exchange.write(rate);
        
        // Determine playback direction
        bool backwards = false;
        if (rate<0.)
            backwards = true;

        //qDebug("myRate: %f, playButton: %f",myRate,playButton->get());
        if (rate==0.)
        {
            for (int i=0; i<buf_size; i++)
                buffer[i]=0.;

            pause.unlock();
            return buffer;
        }
        else
        {
            // Check if we are at the boundaries of the file
            if ((filepos_play<0. && backwards==true) ||
                (filepos_play>file->length() && backwards==false))
            {
                for (int i=0; i<buf_size; i++)
                    buffer[i] = 0.;

                pause.unlock();
                return buffer;
            }

            //double abs_rate = fabs(myRate);
            double rate_add = 2.*rate;

            // Determine position in read_buffer to start from (idx)
            double idx = bufferpos_play;

            // Prepare buffer
            if (backwards)
            {
                for (int i=0; i<buf_size; i+=2)
                {
                    long prev = (long)(floor(idx)+READBUFFERSIZE)%READBUFFERSIZE;
                    if (!even(prev)) prev--;
                    long next = (prev-2+READBUFFERSIZE)%READBUFFERSIZE;
                
                    FLOAT_TYPE frac = idx-floor(idx);
                    buffer[i  ] = read_buffer_prt[prev  ] + frac*(read_buffer_prt[next  ]-read_buffer_prt[prev  ]);
                    buffer[i+1] = read_buffer_prt[prev+1] + frac*(read_buffer_prt[next+1]-read_buffer_prt[prev+1]);

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
                    buffer[i  ] = read_buffer_prt[prev  ] + frac*(read_buffer_prt[next  ]-read_buffer_prt[prev  ]);
                    buffer[i+1] = read_buffer_prt[prev+1] + frac*(read_buffer_prt[next+1]-read_buffer_prt[prev+1]);

                    idx += rate_add;
                }
            }
        
            // Write buffer playpos (ensure valid range)
            if (idx>READBUFFERSIZE)
                idx -= (double)READBUFFERSIZE;
            else if (idx<0)
                idx += (double)READBUFFERSIZE;
            bufferpos_play = idx;
        
            // Write file playpos
            filepos_play += rate*(double)buf_size;
        
            // Try to write playpos for exchange with other threads
            filepos_play_exchange.write(filepos_play);
        
            // Update visual rate and playpos
            visualPlaypos = (int)floor(idx);
            visualRate = rate;
        }
        checkread(backwards);
        pause.unlock();
    }
    else
    {
        for (int i=0; i<buf_size; i++)
            buffer[i]=0.;
    }

    return buffer;
}
