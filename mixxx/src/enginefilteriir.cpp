/***************************************************************************
                          enginefilteriir.cpp  -  description
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

#include "enginefilteriir.h"

EngineFilterIIR::EngineFilterIIR(const float *coefs)
{
    this->coefs = coefs;

    // Reset the yv's:
    for (int i=0; i<=NPOLES; ++i)
        yv[i]=xv[i]=0;
}

EngineFilterIIR::~EngineFilterIIR() 
{
}

void EngineFilterIIR::process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize) 
{
    CSAMPLE *pOutput = (CSAMPLE *)pOut;
    float GAIN =  coefs[0];
    int i;
    for (i=0; i<iBufferSize; ++i) 
    {
        xv[0] = xv[1]; xv[1] = xv[2]; xv[2] = xv[3]; xv[3] = xv[4];
        xv[4] = xv[5]; xv[5] = xv[6]; xv[6] = xv[7]; xv[7] = xv[8]; 
        xv[8] = pIn[i]/GAIN;
        yv[0] = yv[1]; yv[1] = yv[2]; yv[2] = yv[3]; yv[3] = yv[4];
        yv[4] = yv[5]; yv[5] = yv[6]; yv[6] = yv[7]; yv[7] = yv[8]; 
  
        yv[8] =   (xv[0] + xv[8]) + coefs[1] * (xv[1] + xv[7]) + 
                  coefs[2] * (xv[2] + xv[6]) + 
                  coefs[3] * (xv[3] + xv[5]) + coefs[4] * xv[4] + 
                  (coefs[5] * yv[0]) + ( coefs[6] * yv[1]) + 
                  (coefs[7] * yv[2]) + ( coefs[8] * yv[3]) + 
                  (coefs[9] * yv[4]) + ( coefs[10] * yv[5]) + 
                  (coefs[11] * yv[6]) + ( coefs[12] * yv[7]);
  
        ASSERT(yv[8]<100000 || yv[8]>-100000);
          
        pOutput[i] = yv[8];
    }

    // Check for denormals
#ifndef __MACX__
    for (i=0; i<9; ++i)
    {
        xv[i] = zap_denormal(xv[i]);
        yv[i] = zap_denormal(yv[i]);
    }
#endif
}
