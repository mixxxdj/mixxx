/***************************************************************************
                          enginechannel.cpp  -  description
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

#include "enginechannel.h"
#include "controlengine.h"
#include "controlpushbutton.h"
#include "engineclipping.h"
#include "enginepregain.h"
#include "enginevolume.h"
#include "enginefilterblock.h"
#include "enginevumeter.h"

EngineChannel::EngineChannel(const char *group)
{
    // Pregain:
    pregain = new EnginePregain(group);

    // Filters:
    filter = new EngineFilterBlock(group);

    // Clipping:
    clipping = new EngineClipping(group);

    // Volume control:
    volume = new EngineVolume(ConfigKey(group,"volume"));

    // VU meter:
    vumeter = new EngineVuMeter(group);

    // PFL button
    ControlPushButton *p = new ControlPushButton(ConfigKey(group, "pfl" ));
    pfl = new ControlEngine(p);
}

EngineChannel::~EngineChannel()
{
    delete pregain;
    delete filter;
    delete clipping;
    delete volume;
}

ControlEngine *EngineChannel::getPFL()
{
    return pfl;
}

void EngineChannel::process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize)
{
    pregain->process(pIn, pOut, iBufferSize);
    clipping->process(pOut, pOut, iBufferSize);
    filter->process(pOut, pOut, iBufferSize); 
    volume->process(pOut, pOut, iBufferSize);
    vumeter->process(pOut, pOut, iBufferSize);
}
