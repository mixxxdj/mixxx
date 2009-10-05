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
    float volume=(float)potmeter->get();

    if (volume == 1.)
    {
        if (pIn!=pOut)
        {
            for (int i=0; i<iBufferSize; ++i)
                pOutput[i] = pIn[i];
            //memcpy(pOutput, pIn, sizeof(CSAMPLE) * iBufferSize);
        }
    }
    else
    {
        SampleUtil::copyWithGain(pOutput, pIn, volume, iBufferSize);
        // for (int i=0; i<iBufferSize; ++i)
        //     pOutput[i] = pIn[i]*volume;
    }
}
