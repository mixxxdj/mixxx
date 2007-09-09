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

#include "fakemonitor.h"

FakeMonitor::FakeMonitor(double v)
{
    value = v;
}

FakeMonitor::FakeMonitor()
{
    value = 0.;
}

FakeMonitor::~FakeMonitor()
{
}

double FakeMonitor::read()
{
    double temp;
    temp = value;
    return temp;
}

void FakeMonitor::write(double v)
{
    value = v;
}

void FakeMonitor::add(double v)
{
    value += v;
}
