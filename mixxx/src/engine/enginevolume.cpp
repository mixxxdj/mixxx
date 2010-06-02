/***************************************************************************
                          enginevolume.cpp  -  description
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

#include "enginevolume.h"
#include "controllogpotmeter.h"
#include "configobject.h"
#include "sampleutil.h"

/*----------------------------------------------------------------
   Volume effect.
   ----------------------------------------------------------------*/
EngineVolume::EngineVolume(ConfigKey key, double maxval)
{
    potmeter = new ControlLogpotmeter(key, maxval);
}

EngineVolume::~EngineVolume()
{
    delete potmeter;
}

void EngineVolume::process(const CSAMPLE * pIn, const CSAMPLE * pOut, const int iBufferSize)
{
    CSAMPLE * pOutput = (CSAMPLE *)pOut;
    double volume = potmeter->get();

    // SampleUtil handles aliased buffers and gains of 1 or 0.
    SampleUtil::copyWithGain(pOutput, pIn, volume, iBufferSize);
}
