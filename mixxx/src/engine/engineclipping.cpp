/***************************************************************************
                          engineclipping.cpp  -  description
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

#include "engineclipping.h"
#include "controlpotmeter.h"

/*----------------------------------------------------------------
   A pregaincontrol is ... a pregain.
   ----------------------------------------------------------------*/
EngineClipping::EngineClipping(const char * group)
{
    //Used controlpotmeter as the example used it :/ perhaps someone with more knowledge could use something more suitable...
    m_ctrlClipping = new ControlPotmeter(ConfigKey(group, "PeakIndicator"), 0., 1.);
    m_ctrlClipping->set(0);
}

EngineClipping::~EngineClipping()
{
    delete m_ctrlClipping;
}

void EngineClipping::process(const CSAMPLE * pIn, const CSAMPLE * pOut, const int iBufferSize)
{
    static const FLOAT_TYPE kfMaxAmp = 32767.;
    static const FLOAT_TYPE kfClip = 0.8*kfMaxAmp;

    CSAMPLE * pOutput = (CSAMPLE *)pOut;
    clipped = false;
    if (pIn==pOut)
    {
        for (int i=0; i<iBufferSize; ++i)
        {
/*
            CSAMPLE fTmp = pIn[i];
            if ((fTmp>kfClip) || (fTmp<-kfClip))
            {
                FLOAT_TYPE sign = 1;
                if (fTmp<0)
                    sign = -1;
                pOutput[i] = sign*(kfMaxAmp - ((kfMaxAmp-kfClip)*(kfMaxAmp-kfClip)) /
                                   ((kfMaxAmp-2.*kfClip)+sign*pIn[i]));
            }
 */

            if (pIn[i]>kfMaxAmp){
                pOutput[i] = kfMaxAmp;
                clipped = true;
            }
            else if (pIn[i]<-kfMaxAmp)
            {
                pOutput[i] = -kfMaxAmp;
                clipped = true;
            }
        }
    }
    else
    {
        for (int i=0; i<iBufferSize; ++i)
        {
/*
            CSAMPLE fTmp = pIn[i];
            if ((fTmp>kfClip) || (fTmp<-kfClip))
            {
                FLOAT_TYPE sign = 1;
                if (fTmp<0)
                    sign = -1;
                pOutput[i] = sign*(kfMaxAmp - ((kfMaxAmp-kfClip)*(kfMaxAmp-kfClip)) /
                                   ((kfMaxAmp-2.*kfClip)+sign*pIn[i]));
            }
            else
                pOutput[i] = pIn[i];
 */
            if (pIn[i]>kfMaxAmp){
                pOutput[i] = kfMaxAmp;
                clipped = true;
            }
            else if (pIn[i]<-kfMaxAmp){
                pOutput[i] = -kfMaxAmp;
                clipped = true;
            }
            else
                pOutput[i] = pIn[i];
        }
    }
    if(clipped)
        m_ctrlClipping->set(1.);
    else
        m_ctrlClipping->set(0);
}

//returns true if the last buffer processed clipped
bool EngineClipping::hasClipped()
{
    return clipped;
}
