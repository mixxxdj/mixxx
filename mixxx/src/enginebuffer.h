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
#include <qapplication.h>
#include <qwidget.h>

#include "defs.h"
#include "monitor.h"
#include "fakemonitor.h"
#include "engineobject.h"
#include "controlpushbutton.h"
#include "controlpotmeter.h"
#include "controlttrotary.h"
#include "dlgplaycontrol.h"
#include "dlgchannel.h"
#include "midiobject.h"
#include "soundsource.h"

class MixxxApp;
class SoundBuffer;

/**
  *@author Tue and Ken Haste Andersen
*/

class EngineBuffer : public EngineObject, public QThread {
 Q_OBJECT
public:
    EngineBuffer(QApplication *app, MixxxApp *mixxx, DlgPlaycontrol *, const char *group, const char *filename);
    ~EngineBuffer();
    void newtrack(const char *);
    CSAMPLE *update_visual();
    CSAMPLE *process(const CSAMPLE *, const int);
    const char *getGroup();
    SoundBuffer *getSoundBuffer();
    
    /** Playpos slider values */
    FLOAT_TYPE playposSliderLast, playposSliderNew;
    /** Buffer used in the process() */
    CSAMPLE *buffer;

    int visualPlaypos;
    float visualRate;
public slots:
   void slotUpdatePlay(valueType);
   void slotUpdateRate(FLOAT_TYPE);
   void slotPosition(int);
signals:
   void position(int);   
private:

    void run();
    void stop();
    QSemaphore *requestStop;
    QWaitCondition *buffersReadAhead;
    QSemaphore *readChunkLock;

   QApplication *app;
   MixxxApp *mixxx;
   bool pause;
   int start_seek;
   Monitor rate;

   SoundBuffer *soundbuffer;
   
   /** The current sample to play in the file. */
   Monitor filepos_play; 
   /** Sample in the buffer relative to filepos_play. */
   Monitor bufferpos_play;

   DlgPlaycontrol *playcontrol;
   
   void seek(FLOAT_TYPE);
   void checkread();
   void writepos();
   ControlPushButton *PlayButton;
   ControlPotmeter *rateSlider;
   ControlTTRotary *wheel;
   SoundSource *file;
   const char *group;

   CSAMPLE *read_buffer_prt;

   /** Rate factor between player and sound source sample rates */
   FLOAT_TYPE BASERATE;
};
#endif
