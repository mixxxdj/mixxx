/***************************************************************************
                          engineflanger.h  -  description
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

#ifndef ENGINEFLANGER_H
#define ENGINEFLANGER_H

#include <QMap>

#include "engineobject.h"

class ControlPotmeter;
class ControlPushButton;

const int max_delay = 5000;

class EngineFlanger : public EngineObject
{
public:
    EngineFlanger(const char *group);
    ~EngineFlanger();
    void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize);
private:
    ControlObject *potmeterDepth, *potmeterDelay, *potmeterLFOperiod;
    ControlPushButton* flangerEnable;
    CSAMPLE *delay_buffer;
    int  LFOamplitude;
    int average_delay_length;
    int time;
    FLOAT_TYPE delay;
    int delay_pos;
};

#endif
