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

/**
  *@author Tue and Ken Haste Andersen
*/

class EngineBuffer : public EngineObject, public QThread  {
 Q_OBJECT
public:
    EngineBuffer(QApplication *app, QWidget *mixxx, DlgPlaycontrol *, const char *group, const char *filename);
    ~EngineBuffer();
    void setVisual(QObject *visualBuffer);
    void newtrack(const char *);
    CSAMPLE *update_visual();
    CSAMPLE *process(const CSAMPLE *, const int);
    const char *getGroup();

    /** Playpos slider values */
    FLOAT_TYPE playposSliderLast, playposSliderNew;
    /** Buffer used in the process() */
    CSAMPLE *buffer; 
    int visualPos1, visualLen1, visualPos2, visualLen2, visualPlaypos;
    float visualRate;
    /** The buffer where the samples are read into */
    CSAMPLE *read_buffer;
public slots:
   void slotUpdatePlay(valueType);
   void slotUpdateRate(FLOAT_TYPE);
   void slotPosition(int);
signals:
   void position(int);   
private:
   QApplication *app;
   QWidget *mixxx;
   bool pause;
   int start_seek;
   Monitor rate;
   void run();
   void stop();
   QSemaphore *requestStop;
   QSemaphore *readChunkLock;
   QWaitCondition *buffersReadAhead;

   /** The first read sample in the file, currently in read_buffer. */
   Monitor filepos_start;
   /** The last read sample in the file, currently in read_buffer. */
   Monitor filepos_end;
   /** The current sample to play in the file. */
   Monitor filepos_play; 
   /** Sample in the buffer relative to filepos_play. */
   Monitor bufferpos_play;
   /** Buffer start and end position */
   int bufferpos_start, bufferpos_end;
   /** Pointer to visual vertex buffer */
   QObject *visualBuffer;

   DlgPlaycontrol *playcontrol;
   
   void getchunk();
   void seek(FLOAT_TYPE);
   void checkread();
   void writepos();
   ControlPushButton *PlayButton;
   ControlPotmeter *rateSlider;
   ControlTTRotary *wheel;
   SoundSource *file;
   SAMPLE *temp;
   const char *group;
};
#endif
