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

#include <time.h>

#include "enginepregain.h"
#include "controllogpotmeter.h"
#include "controlpotmeter.h"
#include "controlpushbutton.h"
#include "configobject.h"
#include "controlobject.h"

#include "sampleutil.h"


/*----------------------------------------------------------------
   A pregaincontrol is ... a pregain.
   ----------------------------------------------------------------*/
EnginePregain::EnginePregain(const char * group)
{
    potmeterPregain = new ControlLogpotmeter(ConfigKey(group, "pregain"), 4.);
    //Replay Gain things
    m_pControlReplayGain = new ControlObject(ConfigKey(group, "replaygain"));
    m_pTotalGain = new ControlObject(ConfigKey(group, "total_gain"));


    if(ControlObject::getControl(ConfigKey("[ReplayGain]", "InitialReplayGainBoost"))==NULL)
    {
        m_pReplayGainBoost = new ControlPotmeter(ConfigKey("[ReplayGain]", "InitialReplayGainBoost"),0., 15.);
        m_pEnableReplayGain = new ControlPotmeter(ConfigKey("[ReplayGain]", "ReplayGainEnabled"));
    }
    else
    {
        m_pReplayGainBoost = (ControlPotmeter*)ControlObject::getControl(ConfigKey("[ReplayGain]", "InitialReplayGainBoost"));
        m_pEnableReplayGain = (ControlPotmeter*)ControlObject::getControl(ConfigKey("[ReplayGain]", "ReplayGainEnabled"));
    }

    m_bSmoothFade = false;
    m_fClock=0;
    m_fSumClock=0;

}

EnginePregain::~EnginePregain()
{
    delete potmeterPregain;
    delete m_pControlReplayGain;
    delete m_pTotalGain;
}

void EnginePregain::process(const CSAMPLE * pIn, const CSAMPLE * pOut, const int iBufferSize)
{

    float fEnableReplayGain = m_pEnableReplayGain->get();
    float fReplayGainBoost = m_pReplayGainBoost->get();
    CSAMPLE * pOutput = (CSAMPLE *)pOut;
    float fGain = potmeterPregain->get();
    float fReplayGain = m_pControlReplayGain->get();
    m_fReplayGainCorrection=1;
    fGain = fGain/2;
    if(fReplayGain*fEnableReplayGain != 0)
    {
        // Here is the point, when ReplayGain Analyser takes its action, suggested gain changes from 0 to a nonzero value
        // We want to smoothly fade to this last.
        // Anyway we have some the problem that code cannot block the full process for one second.
        // So we need to alter gain each time ::process is called.

        if(m_bSmoothFade)//This means that a ReplayGain value has been calculated after the track has been loaded
        {
            if(m_fClock==0)
                m_fClock=clock();
            m_fSumClock += (float)((clock()-m_fClock)/CLOCKS_PER_SEC);
            m_fClock=clock();
            if(m_fSumClock<1)
            {
                //Fade smoothly

                m_fReplayGainCorrection=(1-m_fSumClock)+(m_fSumClock)*fReplayGain*pow(10, fReplayGainBoost/20);

            }
            else
            {
                m_bSmoothFade = false;
            }
        }
        else
        {
            //Passing a user defined boost
            m_fReplayGainCorrection=fReplayGain*pow(10, fReplayGainBoost/20);
        }
    }
    else
    {
        // If track has not ReplayGain value and ReplayGain is enabled
        // we prepare for smoothfading to ReplayGain suggested gain
        if(fEnableReplayGain != 0)
        {
            m_bSmoothFade=true;
            m_fClock=0;
            m_fSumClock=0;
        }
    }
    fGain = fGain*m_fReplayGainCorrection;
    m_pTotalGain -> set(fGain);

    //qDebug()<<"Clock"<<(float)clock()/CLOCKS_PER_SEC;
    // SampleUtil deals with aliased buffers and gains of 1 or 0.
    SampleUtil::copyWithGain(pOutput, pIn, fGain, iBufferSize);
}
