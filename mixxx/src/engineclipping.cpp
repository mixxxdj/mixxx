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

/*----------------------------------------------------------------
  A pregaincontrol is ... a pregain.
  ----------------------------------------------------------------*/
EngineClipping::EngineClipping(const char *)
{
}

EngineClipping::~EngineClipping()
{
}

void EngineClipping::process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize)
{
    static const FLOAT_TYPE kfMaxAmp = 32676.;
    static const FLOAT_TYPE kfClip = 0.8*kfMaxAmp;

    CSAMPLE *pOutput = (CSAMPLE *)pOut;
    
    int iSamplesClipped = 0; 
    
    if (pIn==pOut)
    {
        for (int i=0; i<iBufferSize; ++i) 
        {
            CSAMPLE fTmp = pIn[i];
            if ((fTmp>kfClip) || (fTmp<-kfClip)) 
            {
                FLOAT_TYPE sign = 1;
                if (fTmp<0) 
                    sign = -1;
                pOutput[i] = sign*(kfMaxAmp - ((kfMaxAmp-kfClip)*(kfMaxAmp-kfClip)) /
                                   ((kfMaxAmp-2.*kfClip)+sign*pIn[i]));
                iSamplesClipped++;
            } 
        }
    }
    else
    {
        for (int i=0; i<iBufferSize; ++i) 
        {
            CSAMPLE fTmp = pIn[i];
            if ((fTmp>kfClip) || (fTmp<-kfClip)) 
            {
                FLOAT_TYPE sign = 1;
                if (fTmp<0) 
                    sign = -1;
                pOutput[i] = sign*(kfMaxAmp - ((kfMaxAmp-kfClip)*(kfMaxAmp-kfClip)) /
                                   ((kfMaxAmp-2.*kfClip)+sign*pIn[i]));
                iSamplesClipped++;
            } 
            else
                pOutput[i] = pIn[i];
        }
    }
}
