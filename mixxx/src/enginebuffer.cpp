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
#include <qevent.h>
#include "wplaybutton.h"
#include "wwheel.h"
#include "wknob.h"
#include "wslider.h"
#include "wplayposslider.h"
#include "configobject.h"
#include "mixxxvisual.h"
#include "visual/guichannel.h"
#include "visual/signalvertexbuffer.h"
#include "mixxx.h"
#include "controlpushbutton.h"
#include "controlpotmeter.h"
#include "controlttrotary.h"
#include "controlengine.h"
#include "controlbeat.h"
#include "dlgplaycontrol.h"
#include "reader.h"
#include "readerextractbeat.h"

EngineBuffer::EngineBuffer(MixxxApp *_mixxx, QAction *actionAudioBeatMark, DlgPlaycontrol *_playcontrol, const char *_group)
{
    mixxx = _mixxx;
    playcontrol = _playcontrol;
    group = _group;

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
    playposSlider->setNotify(this,(void (EngineObject::*)(double))seek);

    // BPM control
    ControlBeat *p4 = new ControlBeat(ConfigKey(group, "bpm"));
    bpmControl = new ControlEngine(p4);
//    bpmControl->setNotify(this,(void (EngineObject::*)(double))bpmChange);
                
    // Beat event control
    p2 = new ControlPotmeter(ConfigKey(group, "beatevent"));
    beatEventControl = new ControlEngine(p2);

    // Audio beat mark toggle
    p = new ControlPushButton(ConfigKey(group, "audiobeatmarks"));
    p->setAction(actionAudioBeatMark);
    audioBeatMark = new ControlEngine(p);

    // Control file changed
//    filechanged = new ControlEngine(controlfilechanged);
//    filechanged->setNotify(this,(void (EngineObject::*)(double))newtrack);
   
    reader = new Reader(this, mixxx, &rate_exchange, &pause);
    read_buffer_prt = reader->getBufferWavePtr();
    file_length_old = -1;
    file_srate_old = 0;
    rate_old = 0;
                                                            
    // Allocate buffer for processing:
    buffer = new CSAMPLE[MAX_BUFFER_LEN];

    playposUpdateCounter = 0;
    oldEvent = 0.;

    reader->start();
}

EngineBuffer::~EngineBuffer()
{

    delete playButton;
    delete wheel;
    delete rateSlider;
    delete buffer;
}

Reader *EngineBuffer::getReader()
{
    return reader;
}

void EngineBuffer::setNewPlaypos(double newpos)
{
    filepos_play = newpos;
    bufferpos_play = 0.;

    // Ensures that the playpos slider gets updated in next process call
    playposUpdateCounter = 1000000;
}

const char *EngineBuffer::getGroup()
{
    return group;
}

int EngineBuffer::getPlaypos(int Srate)
{
    return 0; //(int)((CSAMPLE)visualPlaypos.read()/(2.*(CSAMPLE)file_srate/(CSAMPLE)Srate));
}


void EngineBuffer::seek(double change)
{
    qDebug("seeking... %f",change);

    // Find new playpos
    double new_playpos = change*file_length_old;
    if (new_playpos > file_length_old)
        new_playpos = file_length_old;
    if (new_playpos < 0.)
        new_playpos = 0.;

    // Seek reader
    reader->requestSeek(new_playpos);

//    filepos_play_exchange.write(filepos_play);
//    file->seek((long unsigned)filepos_play);
//    visualPlaypos.tryWrite(
}

/*
void EngineBuffer::bpmChange(double bpm)
{
    CSAMPLE filebpm = bpmbuffer[];


    baserate = baserate*(bpm/reader->->getBPM());
}
*/

inline bool even(long n)
{
//    if ((n/2) != (n+1)/2)
    if (n%2 != 0)
        return false;
    else
        return true;
}

CSAMPLE *EngineBuffer::process(const CSAMPLE *, const int buf_size)
{

    if (pause.tryLock())
    {
        // Try to fetch info from the reader
        bool readerinfo = false;
        long int filepos_start, filepos_end;
        if (reader->enginelock.tryLock())
        {
            file_length_old = reader->getFileLength();
            file_srate_old = reader->getFileSrate();
            filepos_start = reader->getFileposStart();
            filepos_end = reader->getFileposEnd();
            reader->enginelock.unlock();
            readerinfo = true;
        }

        //
        // Calculate rate
        //

        // Find bpm adjustment factor
        ReaderExtractBeat *beat = reader->getBeatPtr();
        CSAMPLE *bpmBuffer = beat->getBpmPtr();
        double filebpm = bpmBuffer[(int)(bufferpos_play*(beat->getBufferSize()/READCHUNKSIZE))];
        double bpmrate;
        if (bpmControl->get()>-1. && filebpm>-1.)
            bpmrate = bpmControl->get()/filebpm;
        else
            bpmrate = 1.;
//        qDebug("bpmrate %f, filebpm %f, midibpm %f",bpmrate,filebpm,bpmControl->get());
                    
        double baserate =  bpmrate*((double)file_srate_old/(double)getPlaySrate());
        double rate;
        if (playButton->get()==1.)
            rate=wheel->get()+rateSlider->get()*baserate;
        else
            rate=wheel->get()*baserate*20.;

/*
        //
        // Beat event control. Assume forward play
        //    

        // Search for next beat
        ReaderExtractBeat *readerbeat = reader->getBeatPtr();
        bool *beatBuffer = (bool *)readerbeat->getBasePtr();
        int nextBeatPos;
        int beatBufferPos = bufferpos_play*((CSAMPLE)readerbeat->getBufferSize()/(CSAMPLE)READBUFFERSIZE);
        int i;
        for (i=beatBufferPos+1; i<beatBufferPos+readerbeat->getBufferSize(); i++)
            if (beatBuffer[i%readerbeat->getBufferSize()])
                break;
        if (beatBuffer[i%readerbeat->getBufferSize()])
            // Next beat was found
            nextBeatPos = (i%readerbeat->getBufferSize())*(READBUFFERSIZE/readerbeat->getBufferSize());
        else
            // No next beat was found
            nextBeatPos = bufferpos_play+buf_size;

        double event = beatEventControl->get();
        if (event > 0.)
        {
            qDebug("event: %f, playpos %f, nextBeatPos %i",event,bufferpos_play,nextBeatPos);
            //
            // Play next event
            //

            // Reset beat event control
            beatEventControl->set(0.);

            if (oldEvent>0.)
            {
                // Adjust bufferplaypos
                bufferpos_play = nextBeatPos;

                // Search for a new next beat position
                ReaderExtractBeat *readerbeat = reader->getBeatPtr();
                bool *beatBuffer = (bool *)readerbeat->getBasePtr();

                int beatBufferPos = bufferpos_play*((CSAMPLE)readerbeat->getBufferSize()/(CSAMPLE)READBUFFERSIZE);
                int i;
                for (i=beatBufferPos+1; i<beatBufferPos+readerbeat->getBufferSize(); i++)
                {
    //                qDebug("i %i",i);
                    if (beatBuffer[i%readerbeat->getBufferSize()])
                        break;
                }
                if (beatBuffer[i%readerbeat->getBufferSize()])
                    // Next beat was found
                    nextBeatPos = (i%readerbeat->getBufferSize())*(READBUFFERSIZE/readerbeat->getBufferSize());
                else
                    // No next beat was found
                    nextBeatPos = -1;
            }
            
            oldEvent = 1.;
        }
        else if (oldEvent==0.)
            nextBeatPos = -1;

//        qDebug("NextBeatPos :%i, bufDiff: %i",nextBeatPos,READBUFFERSIZE/readerbeat->getBufferSize());
*/
        
        // If the rate has changed, write it to the rate_exchange monitor
        if (rate != rate_old)
        {
            rate_exchange.tryWrite(rate);
            rate_old = rate;        
        }

        // Determine playback direction
        bool backwards = false;
        if (rate<0.)
            backwards = true;

        //qDebug("rate: %f, playpos: %f",rate,playButton->get());
        if (rate==0.)
        {
            for (int i=0; i<buf_size; i++)
                buffer[i]=0.;

            pause.unlock();
            //app->unlock();
            return buffer;
        }
        else
        {
            // Check if we are at the boundaries of the file
            if ((filepos_play<0. && backwards==true) ||
                (filepos_play>file_length_old && backwards==false))
            {
                for (int i=0; i<buf_size; i++)
                    buffer[i] = 0.;

                pause.unlock();
                //app->unlock();
                return buffer;
            }

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
                
                // Ensure that we only play until the next beat marking
                int i_end;
/*
                if (nextBeatPos==-1)
                    i_end = 0;
                else
                    i_end = max(0,min(buf_size, nextBeatPos-idx));
*/
//                qDebug("start, i_end %i",i_end);


                i_end = buf_size;
                
                int i;
                for (i=0; i<i_end; i+=2)
                {
//                    if (i==0)
//                        qDebug("playpos: %f",idx);

                    long prev = (long)floor(idx)%READBUFFERSIZE;
                    if (!even(prev)) prev--;

                    long next = (prev+2)%READBUFFERSIZE;

                    FLOAT_TYPE frac = idx - floor(idx);
                    buffer[i  ] = read_buffer_prt[prev  ] + frac*(read_buffer_prt[next  ]-read_buffer_prt[prev  ]);
                    buffer[i+1] = read_buffer_prt[prev+1] + frac*(read_buffer_prt[next+1]-read_buffer_prt[prev+1]);

                    idx += rate_add;
                }
                
                // Pad rest of buffer with zeros if necessary
/*
                if (i<buf_size)
                {
//                    qDebug("end");
                    oldEvent = 0.;
                    for (; i<buf_size; i++)
                        buffer[i] = 0.;
                }
*/
            }


            // If a beat occours in current buffer, and if audio marking is enabled, mark it
            if (audioBeatMark->get()==1.)
            {
                ReaderExtractBeat *readerbeat = reader->getBeatPtr();
                bool *beatBuffer = (bool *)readerbeat->getBasePtr();
                int chunkSizeDiff = READBUFFERSIZE/readerbeat->getBufferSize();

                int from = ((bufferpos_play-audioBeatMarkLen)/chunkSizeDiff);
                int to   = (idx                              /chunkSizeDiff);
                for (int i=from; i<=to; i++)
                    if (beatBuffer[i%readerbeat->getBufferSize()])
                    {
                        int j_start = i*chunkSizeDiff;
                        int j_end   = j_start+audioBeatMarkLen;
//                        qDebug("%i-%i, buffer: %f-%f",j_start,j_end,bufferpos_play,idx);
                        if (j_start > bufferpos_play-audioBeatMarkLen)
                        {
                            j_start = max(0,j_start-bufferpos_play);
                            j_end = min(j_end-bufferpos_play,buf_size);
//                            qDebug("j_start %i, j_end %i",j_start,j_end);
                            for (int j=j_start; j<j_end; j++)
                                buffer[j] = 30000.;
                        }
                    }
            }

            // Ensure valid range of idx
            if (idx>READBUFFERSIZE)
                idx -= (double)READBUFFERSIZE;
            else if (idx<0)
                idx += (double)READBUFFERSIZE;
//if (idx<bufferpos_play)
//    qDebug("idx: %f, bufferpos_play %f, diff: %f",idx,bufferpos_play,bufferpos_play-idx);
        
            // Write file playpos
            filepos_play += rate*(double)(buf_size);;

            // Write buffer playpos
            bufferpos_play = idx;
            
            //qDebug("bufferpos_play %f,\t filepos_play %f", bufferpos_play, filepos_play);

        
            // Try to write playpos for exchange with other threads
            //filepos_play_exchange.write(filepos_play);
        
            // Update visual rate and playpos
            //visualPlaypos.tryWrite(flREADCHUNKSIZE*(READCHUNK_NO/2-2)oor(idx));
            //visualRate = rate;
        }

        //
        // Check if more samples are needed from reader, and wake it up if necessary.
        //
        if (readerinfo)
        {
            //qDebug("check len %i, end %i, play %f, size %i",file_length_old, filepos_end, filepos_play, READCHUNKSIZE*(READCHUNK_NO/2-2));
/*
            if (controlreader...)
            {
                buffersReadAhead->wakeAll();
            }
            else
*/
            if (!backwards && filepos_end< file_length_old && (filepos_end - filepos_play < READCHUNKSIZE*(READCHUNK_NO/2-2)))
                reader->wake();
            else if (backwards && filepos_start>0. && (filepos_play - filepos_start < READCHUNKSIZE*(READCHUNK_NO/2-2)))
                reader->wake();

            //
            // Check if end or start of file, and playmode, write new rate, playpos and do wakeall
            // if playmode is next file: set next in playlistcontrol
            //

            // Update playpos slider if necessary
            playposUpdateCounter +=buf_size;
            if (playposUpdateCounter>(int)(file_length_old/(127.*rate)))
            {
                playposSlider->set(filepos_play/file_length_old);
                playposUpdateCounter = 0;
            }
        }
        pause.unlock();

    }
    else
    {
        for (int i=0; i<buf_size; i++)
            buffer[i]=0.;
    }

    return buffer;
}
