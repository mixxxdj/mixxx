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

double Monitor::tryRead()
{
    double temp;
    if (mutex.tryLock())
    {
        temp = value;
        mutex.unlock();
    }
    else
        temp = -1.;
    return temp;
}

void Monitor::tryWrite(double v)
{
    if (mutex.tryLock())
    {
        value = v;
        mutex.unlock();
    }
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