/***************************************************************************
                          engineobject.cpp  -  description
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

#include "engineobject.h"

// Static member variable definition
QString EngineObject::NAME = 0;
int EngineObject::SRATE = 0;
int EngineObject::BITS = 0;
int EngineObject::BUFFERSIZE = 0;
int EngineObject::NYQUIST = 0;
CSAMPLE EngineObject::norm = 0.;
MixxxView *EngineObject::view = 0;

EngineObject::EngineObject()
{
    //view = v;
}

EngineObject::~EngineObject()
{
}

void EngineObject::setParams(QString name, int srate, int bits, int bufferSize)
{
    NAME       = name;
    SRATE      = srate;
    BITS       = bits;
    BUFFERSIZE = bufferSize;

    NYQUIST = SRATE/2;
    norm    = (2.*acos(-1.0))/SRATE;
}

