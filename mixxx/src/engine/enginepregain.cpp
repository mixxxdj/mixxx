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

#include <QtDebug>

#include "engine/enginepregain.h"
#include "controllogpotmeter.h"
#include "controlpotmeter.h"
#include "controlpushbutton.h"
#include "configobject.h"
#include "controlobject.h"

#include "sampleutil.h"
#include <time.h>   // for clock() and CLOCKS_PER_SEC

ControlPotmeter* EnginePregain::s_pReplayGainBoost = NULL;
ControlObject* EnginePregain::s_pEnableReplayGain = NULL;

/*----------------------------------------------------------------
   A pregaincontrol is ... a pregain.
   ----------------------------------------------------------------*/
EnginePregain::EnginePregain(const char * group)
{
    potmeterPregain = new ControlLogpotmeter(ConfigKey(group, "pregain"), 4.);
    //Replay Gain things
    m_pControlReplayGain = new ControlObject(ConfigKey(group, "replaygain"));
    m_pTotalGain = new ControlObject(ConfigKey(group, "total_gain"));

    if (s_pReplayGainBoost == NULL) {
        s_pReplayGainBoost = new ControlPotmeter(ConfigKey("[ReplayGain]", "InitialReplayGainBoost"),0., 15.);
        s_pEnableReplayGain = new ControlObject(ConfigKey("[ReplayGain]", "ReplayGainEnabled"));
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

    delete s_pEnableReplayGain;
    s_pEnableReplayGain = NULL;
    delete s_pReplayGainBoost;
    s_pReplayGainBoost = NULL;
}

void EnginePregain::process(const CSAMPLE * pIn, const CSAMPLE * pOut, const int iBufferSize)
{

    float fEnableReplayGain = s_pEnableReplayGain->get();
    float fReplayGainBoost = s_pReplayGainBoost->get();
    CSAMPLE * pOutput = (CSAMPLE *)pOut;
    float fGain = potmeterPregain->get();
    float fReplayGain = m_pControlReplayGain->get();
    m_fReplayGainCorrection=1;
    // TODO(XXX) Why do we do this? Removing it results in clipping at unity
    // gain so I think it was trying to compensate for some issue when we added
    // replaygain but even at unity gain (no RG) we are clipping. rryan 5/2012
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
    m_pTotalGain->set(fGain);

    // Clamp gain to within [0, 2.0] to prevent insane gains. This can happen
    // (some corrupt files get really high replaygain values).
    fGain = math_max(0.0, math_min(2.0, fGain));

    //qDebug()<<"Clock"<<(float)clock()/CLOCKS_PER_SEC;
    // SampleUtil deals with aliased buffers and gains of 1 or 0.
    SampleUtil::copyWithGain(pOutput, pIn, fGain, iBufferSize);
}
