/***************************************************************************
                          engineobject.h  -  description
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

#ifndef ENGINEOBJECT_H
#define ENGINEOBJECT_H

#include <qobject.h>
#include "mixxxview.h"
#include "defs.h"

#ifdef __MACX__
  #include <mach/mach_init.h>
  #include <mach/task_policy.h>
  #include <mach/thread_act.h>
  #include <mach/thread_policy.h>
  #include <sys/sysctl.h>
#endif
#ifdef __UNIX__
  #include <sched.h>
  #include <sys/time.h>
  #include <sys/resource.h>
#endif


/**
  *@author Tue and Ken Haste Andersen
  */

class EngineObject : public QObject  {
  Q_OBJECT
public: 
  EngineObject();
  ~EngineObject();
  virtual CSAMPLE *process(const CSAMPLE *, const int) = 0;

  static QString NAME_MASTER;
  static QString NAME_HEAD;
  static int SRATE;
  static int BITS;
  static int BUFFERSIZE;
  static int CH_MASTER;
  static int CH_HEAD;
  static int NYQUIST;
  static CSAMPLE norm;
protected:
  void setParams(QString name, int srate, int bits, int bufferSize, int chMaster, int chHead);
  static FLOAT_TYPE BASERATE;
#ifdef __MACX__
  int get_bus_speed();
#endif
  void rtThread();
private:
  static MixxxView *view;
};

#endif
