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
#include "controlpushbutton.h"
#include "engineclipping.h"
#include "enginepregain.h"
#include "enginevolume.h"
#include "enginefilterblock.h"
#include "enginevumeter.h"
#include "enginefilteriir.h"
#include "enginetemporal.h"

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
    pfl = new ControlPushButton(ConfigKey(group, "pfl"), true);

    // Temporal filtering
    m_pEngineTemporalVolume = new EngineVolume(ConfigKey(group, "temporalVolume"));
    m_pEngineTemporal = new EngineTemporal(group, m_pEngineTemporalVolume);
}

EngineChannel::~EngineChannel()
{
    delete pregain;
    delete filter;
    delete clipping;
    delete volume;
    delete pfl;
    delete m_pEngineTemporalVolume;
    delete m_pEngineTemporal;
}

void EngineChannel::setVisual(EngineBuffer *pEngineBuffer)
{
    Q_ASSERT(pEngineBuffer);
#ifdef TEMPORAL
    m_pEngineTemporal->addVisual(pEngineBuffer);
#endif
}

ControlPushButton *EngineChannel::getPFL()
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

#ifdef TEMPORAL
    // Temporal filtering
    m_pEngineTemporal->process(pOut, pOut, iBufferSize);
#endif
}
