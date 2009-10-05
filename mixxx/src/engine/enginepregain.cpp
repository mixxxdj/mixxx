/***************************************************************************
                          enginepregain.cpp  -  description
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

#include "enginepregain.h"
#include "controllogpotmeter.h"
#include "sampleutil.h"

/*----------------------------------------------------------------
   A pregaincontrol is ... a pregain.
   ----------------------------------------------------------------*/
EnginePregain::EnginePregain(const char * group)
{
  potmeterPregain = new ControlLogpotmeter(ConfigKey(group, "pregain"), 4.);
  //  potmeterPregain = new ControlPotmeter(ConfigKey(group, "pregain"), -1., 1.);

}

EnginePregain::~EnginePregain()
{
    delete potmeterPregain;
}

void EnginePregain::process(const CSAMPLE * pIn, const CSAMPLE * pOut, const int iBufferSize)
{
    CSAMPLE * pOutput = (CSAMPLE *)pOut;
    float fGain = potmeterPregain->get();
    fGain = fGain/2;

    // SampleUtil deals with aliased buffers and gains of 1 or 0.
    SampleUtil::addWithGain(pOutput, pIn, fGain, iBufferSize);

    // if (fGain == 1.)
    // {
    //     if (pIn!=pOut)
    //     {
    //         for (int i=0; i<iBufferSize; ++i)
    //             pOutput[i] = pIn[i];
    //         //memcpy(pOutput, pIn, sizeof(CSAMPLE) * iBufferSize);
    //     }
    //     return;
    // }
    // for (int i=0; i<iBufferSize; ++i)
    //     pOutput[i] = pIn[i]*fGain;
}
