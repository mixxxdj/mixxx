/***************************************************************************
                          rtthread.h  -  description
                             -------------------
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

/* Set real-time priority on a calling thread.
 *
 * Tue Haste Andersen <haste@diku.dk>, 2003.
 */

#ifndef RTTHREAD_H
#define RTTHREAD_H

#ifdef __APPLE__
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

#ifdef __APPLE__
  int get_bus_speed();
#endif

void rtThread();

#endif
