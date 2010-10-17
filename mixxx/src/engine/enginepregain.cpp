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
#include "controlobject.h"
#include "controlpushbutton.h"

/*----------------------------------------------------------------
   A pregaincontrol is ... a pregain.
   ----------------------------------------------------------------*/
EnginePregain::EnginePregain(const char * group)
{
  potmeterPregain = new ControlLogpotmeter(ConfigKey(group, "pregain"), 4.);



}

EnginePregain::~EnginePregain()
{
    delete potmeterPregain;
//    delete m_pFileRG;
//    delete m_pButtonRGApply;
}

void EnginePregain::process(const CSAMPLE * pIn, const CSAMPLE * pOut, const int iBufferSize)
{
    CSAMPLE * pOutput = (CSAMPLE *)pOut;
    float fGain=potmeterPregain->get();
    fGain = fGain/2;
    if (fGain == 1.)
    {
        if (pIn!=pOut)
        {
            for (int i=0; i<iBufferSize; ++i)
                pOutput[i] = pIn[i];
            //memcpy(pOutput, pIn, sizeof(CSAMPLE) * iBufferSize);
        }
        return;
    }

    for (int i=0; i<iBufferSize; ++i)
        pOutput[i] = pIn[i]*fGain;
}
