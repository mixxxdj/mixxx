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
  //Replay Gain things
  ControlReplayGain = new ControlObject(ConfigKey(group, "replaygain"));


}

EnginePregain::~EnginePregain()
{
    delete potmeterPregain;
    delete ControlReplayGain;
//    delete m_pFileRG;
//    delete m_pButtonRGApply;
}

void EnginePregain::process(const CSAMPLE * pIn, const CSAMPLE * pOut, const int iBufferSize)
{
    CSAMPLE * pOutput = (CSAMPLE *)pOut;
    float fGain=potmeterPregain->get();
    float fRGain = ControlReplayGain->get();
    p_fReplayGainCorrection=1;
   if(fRGain != -32767)
   {
    	//Passing from Db to relative peak value with a boost of +9Db
	    //This last should be user configurable  (see http://replaygain.hydrogenaudio.org/)
	   p_fReplayGainCorrection=pow(10,(9+fRGain)/20);
    }
    fGain = (fGain/2)*p_fReplayGainCorrection;;
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
