/***************************************************************************
                          monitor.cpp  -  description
                             -------------------
    begin                : Sun Apr 28 2002
    copyright            : (C) 2002 by
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

#include "monitor.h"

Monitor::Monitor(double v)
{
    value = v;
}

Monitor::Monitor()
{
    value = 0.;
}

Monitor::~Monitor()
{
}

double Monitor::read()
{
    mutex.lock();
    double temp = value;
    mutex.unlock();
    return temp;
}

void Monitor::write(double v)
{
    mutex.lock();
    value = v;
    mutex.unlock();
}

bool Monitor::tryRead(double * v)
{
    if (mutex.tryLock())
    {
        *v = value;
        mutex.unlock();
        return true;
    }
    else
        return false;
}

bool Monitor::tryWrite(double v)
{
    if (mutex.tryLock())
    {
        value = v;
        mutex.unlock();
        return true;
    }
    else
        return false;
}

/*
   void Monitor::add(double v)
   {
    mutex.lock();
    value += v;
    mutex.unlock();
   }

   void Monitor::sub(double v)
   {
    mutex.lock();
    value -= v;
    mutex.unlock();
   }
 */

