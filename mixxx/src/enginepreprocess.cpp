/***************************************************************************
                          enginepreprocess.cpp  -  description
                             -------------------
    begin                : Mon Feb 3 2003
    copyright            : (C) 2003 by Tue and Ken Haste Andersen
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

#include "enginepreprocess.h"
#include "configobject.h"
#include "soundbuffer.h"

EnginePreProcess::EnginePreProcess(SoundBuffer *_soundbuffer, int _specNo, int windowSize)
{
    specNo = _specNo;
    soundbuffer = _soundbuffer;

    // Allocate list of EngineSpectralFwd objects, corresponding to one object for each
    // stepsize throughout the readbuffer of EngineBuffer
    specList.setAutoDelete(TRUE);
    for (int i=0; i<specNo; i++)
        specList.append(new EngineSpectralFwd(true,false,windowSize));
}

EnginePreProcess::~EnginePreProcess()
{
}

void EnginePreProcess::update(int specFrom, int specTo)
{
    if (specTo>specFrom)
        for (int i=specFrom; i<specTo; i++)
            specList.at(i)->process(soundbuffer->getWindowPtr(i),0);
    else
    {
        for (int i=specFrom; i<specNo; i++)
            specList.at(i)->process(soundbuffer->getWindowPtr(i),0);
        for (int i=0; i<specTo; i++)
            specList.at(i)->process(soundbuffer->getWindowPtr(i),0);
    }
}

CSAMPLE *EnginePreProcess::process(const CSAMPLE *, const int)
{
    return 0;
}
