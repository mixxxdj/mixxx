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
#include <qslider.h>
#include "wknob.h"
#include <qstring.h>
#include <qlcdnumber.h>

#include "defs.h"
#include "monitor.h"
#include "fakemonitor.h"
#include "engineobject.h"
#include "controlpushbutton.h"
#include "controlpotmeter.h"
#include "controlrotary.h"
#include "dlgplaycontrol.h"
#include "dlgchannel.h"
#include "midiobject.h"
#include "soundsourceheavymp3.h"
#include "soundsourcemp3.h"
#include "soundsourceaflibfile.h"

/**
  *@author Tue and Ken Haste Andersen
*/

class EngineBuffer : public EngineObject, public QThread  {
 Q_OBJECT
public:
  EngineBuffer(DlgPlaycontrol *, const char *group, const char *filename);
  ~EngineBuffer();
  void newtrack(const char *);
  void start();
  CSAMPLE *process(const CSAMPLE *, const int);
public slots:
   void slotUpdatePlay(valueType);
   void slotUpdateRate(FLOAT_TYPE);
   void slotPosition(int);
   void slotCenterWheel();
   void slotSetWheel(int val);
signals:
   void position(int);   
private:
   bool pause;
   int start_seek;
   Monitor rate;
   Monitor readChunkLock;
   void run();
   void stop();
   QSemaphore *requestStop;
   QWaitCondition *buffersReadAhead;

   Monitor lastread_file; // The last read sample in the file.
   Monitor playpos_file; // The current sample to play in the file.
   Monitor playpos_buffer; // The corresponding sample in the buffer.
   CSAMPLE *read_buffer; // The buffer where the samples are read into
   unsigned long int read_buffer_size; // Length of buffer.

   DlgPlaycontrol *playcontrol;
   
   void getchunk();
   void seek(FLOAT_TYPE);
   void checkread();
   void writepos();
   int end_seek();
   ControlPushButton *PlayButton;
   ControlPotmeter *rateSlider;
   ControlRotary *wheel;
   SoundSource *file;
   SAMPLE *temp;
   unsigned  chunk_size;
   CSAMPLE *buffer; // Buffer used in the process()
   FLOAT_TYPE lastwrite;
};
#endif
