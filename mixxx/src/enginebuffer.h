/***************************************************************************
                          enginebuffer.h  -  description
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

#ifndef ENGINEBUFFER_H
#define ENGINEBUFFER_H

#include <qthread.h>
#include <qwaitcondition.h>
#include <qmutex.h>
#include <qwidget.h>

#include "defs.h"
#include "monitor.h"
#include "fakemonitor.h"
#include "engineobject.h"
#include "dlgplaycontrol.h"
#include "dlgchannel.h"
#include "midiobject.h"
#include "soundsource.h"

class MixxxApp;
class SoundBuffer;
class GUIChannel;
class ControlEngine;
#include "controlpushbutton.h"
#include "controlpotmeter.h"
#include "controlttrotary.h"
#include "controlpushbutton.h"
#include "controlpotmeter.h"
#include "controlttrotary.h"
#include "controlpushbutton.h"
#include "controlpotmeter.h"
#include "controlttrotary.h"
#include "controlpushbutton.h"
#include "controlpotmeter.h"
#include "controlttrotary.h"

/**
  *@author Tue and Ken Haste Andersen
*/

class EngineBuffer : public EngineObject, public QThread {
public:
    EngineBuffer(MixxxApp *_mixxx, DlgPlaycontrol *_playcontrol, const char *_group, const char *filename);
    ~EngineBuffer();
    void newtrack(const char *);
    CSAMPLE *update_visual();
    CSAMPLE *process(const CSAMPLE *, const int);
    const char *getGroup();
    SoundBuffer *getSoundBuffer();
    /** Get the relative playpos in a buffer sampled at Srate hz*/
    int getPlaypos(int Srate);
    /** Notify used to call seek when playpos slider changes */
    void notify(double change);
        
    /** Buffer used in the process() */
    CSAMPLE *buffer;

    int visualPlaypos;
    float visualRate;
private:
    void run();
    void stop();
    void seek(FLOAT_TYPE);
    void checkread(bool backwards);
    
    QSemaphore *requestStop;
    QWaitCondition *buffersReadAhead;
    QSemaphore *readChunkLock;

    MixxxApp *mixxx;
    int start_seek;

    

    SoundBuffer *soundbuffer;
   
    /** The current sample to play in the file. */
    double filepos_play;
    /** Sample in the buffer relative to filepos_play. */
    double bufferpos_play;
    /** Holds the rate as calculated in process. This is used in the
      * reader thread when calling soundbuffer->getchunk() */
    Monitor rate_exchange;
    /** Fileposition used in communication with file reader and main thread */
    Monitor filepos_play_exchange;
    /** Mutex controlling weather the process function is in pause mode. This happens
      * during seek and loading of a new track */
    QMutex pause;
    DlgPlaycontrol *playcontrol;
   
    ControlEngine *playButton, *rateSlider, *wheel, *playposSlider;
    SoundSource *file;
    const char *group;

    CSAMPLE *read_buffer_prt;

    /** Rate factor between player and sound source sample rates */
    FLOAT_TYPE BASERATE;
    /** Pointer to GUIChannel */
    GUIChannel *guichannel;
};
#endif
