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
#include <qpushbutton.h>
#include <qmultilineedit.h>
#include <qslider.h>
#include <qstring.h>

#include <pthread.h>
#include <semaphore.h>

#include "defs.h"
#include "engineobject.h"
#include "soundsource.h"
#include "controlpushbutton.h"
#include "controlpotmeter.h"
#include "controlrotary.h"
#include "dlgplaycontrol.h"
#include "dlgchannel.h"
#include "dlgmessages.h"
#include "midiobject.h"
/**
  *@author Tue and Ken Haste Andersen
  */

class EngineBuffer : public EngineObject, public QThread  {
 Q_OBJECT
private:
  void run();

  ControlPushButton *PlayButton;
  ControlPotmeter *rateSlider;
  ControlRotary *wheel;
  double rate;
  SoundSource *file;
  SAMPLE *temp;
  unsigned  chunk_size;
  unsigned long int filepos;
  FLOAT filelength;
  int direction;
  long distance(const long, const long);
  FLOAT min(const FLOAT, const FLOAT);
  FLOAT max(const FLOAT, const FLOAT);
public:
  EngineBuffer(DlgPlaycontrol *, DlgChannel *, MidiObject *, char *);
  ~EngineBuffer();
  void start();
  void process(CSAMPLE *, CSAMPLE *, int);
  sem_t *buffers_read_ahead;
  unsigned long int read_buffer_size;
  unsigned long int frontpos;
  double play_pos;
  CSAMPLE *readbuffer;
  void getchunk();
  void seek(FLOAT);
  void checkread();
  void writepos();
public slots:
   void slotUpdatePlay(valueType);
   void slotUpdateRate(FLOAT);
};
#endif
