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

#include "engine/enginefilteriir.h"
#include "util/counter.h"

#ifdef _MSC_VER
    #include <float.h>  // for _isnan() on VC++
    #define isnan(x) _isnan(x)  // VC++ uses _isnan() instead of isnan()
#else
    #include <math.h>  // for isnan() everywhere else
#endif

EngineFilterIIR::EngineFilterIIR(const double * pCoefs, int iOrder)
{
    order = iOrder;
    coefs = pCoefs;


    // Reset the yv's:
    memset(yv1, 0, sizeof(yv1));
    memset(yv2, 0, sizeof(yv2));
    memset(xv1, 0, sizeof(xv1));
    memset(xv2, 0, sizeof(xv2));
}

EngineFilterIIR::~EngineFilterIIR()
{
}

void EngineFilterIIR::process(const CSAMPLE * pIn, const CSAMPLE * pOut, const int iBufferSize)
{
    CSAMPLE * pOutput = (CSAMPLE *)pOut;
    double GAIN =  coefs[0];
    int i;
    for (i=0; i<iBufferSize; i+=2)
    {
        if (order==8)
        {
            //8th order:
            // Channel 1
            xv1[0] = xv1[1]; xv1[1] = xv1[2]; xv1[2] = xv1[3]; xv1[3] = xv1[4];
            xv1[4] = xv1[5]; xv1[5] = xv1[6]; xv1[6] = xv1[7]; xv1[7] = xv1[8];
            xv1[8] = pIn[i]/GAIN;
            yv1[0] = yv1[1]; yv1[1] = yv1[2]; yv1[2] = yv1[3]; yv1[3] = yv1[4];
            yv1[4] = yv1[5]; yv1[5] = yv1[6]; yv1[6] = yv1[7]; yv1[7] = yv1[8];
            yv1[8] =   (xv1[0] + xv1[8]) + coefs[1] * (xv1[1] + xv1[7]) +
                     coefs[2] * (xv1[2] + xv1[6]) +
                     coefs[3] * (xv1[3] + xv1[5]) + coefs[4] * xv1[4] +
                     (coefs[5] * yv1[0]) + ( coefs[6] * yv1[1]) +
                     (coefs[7] * yv1[2]) + ( coefs[8] * yv1[3]) +
                     (coefs[9] * yv1[4]) + ( coefs[10] * yv1[5]) +
                     (coefs[11] * yv1[6]) + ( coefs[12] * yv1[7]);
            // Guard against nan.
            if (isnan(yv1[8])) {
                Counter count("EngineFilterIIR::process yv1[8] isnan");
                count.increment();
                yv1[8] = 0;
            }
            pOutput[i] = yv1[8];

            // Channel 2
            xv2[0] = xv2[1]; xv2[1] = xv2[2]; xv2[2] = xv2[3]; xv2[3] = xv2[4];
            xv2[4] = xv2[5]; xv2[5] = xv2[6]; xv2[6] = xv2[7]; xv2[7] = xv2[8];
            xv2[8] = pIn[i+1]/GAIN;
            yv2[0] = yv2[1]; yv2[1] = yv2[2]; yv2[2] = yv2[3]; yv2[3] = yv2[4];
            yv2[4] = yv2[5]; yv2[5] = yv2[6]; yv2[6] = yv2[7]; yv2[7] = yv2[8];
            yv2[8] =   (xv2[0] + xv2[8]) + coefs[1] * (xv2[1] + xv2[7]) +
                     coefs[2] * (xv2[2] + xv2[6]) +
                     coefs[3] * (xv2[3] + xv2[5]) + coefs[4] * xv2[4] +
                     (coefs[5] * yv2[0]) + ( coefs[6] * yv2[1]) +
                     (coefs[7] * yv2[2]) + ( coefs[8] * yv2[3]) +
                     (coefs[9] * yv2[4]) + ( coefs[10] * yv2[5]) +
                     (coefs[11] * yv2[6]) + ( coefs[12] * yv2[7]);
            // Guard against nan.
            if (isnan(yv2[8])) {
                Counter count("EngineFilterIIR::process yv2[8] isnan");
                count.increment();
                yv2[8] = 0;
            }
            pOutput[i+1] = yv2[8];
        }
        else if (order==2)
        {
            // Second order
            xv1[0] = xv1[1]; xv1[1] = xv1[2];
            xv1[2] = pIn[i] / GAIN;
            yv1[0] = yv1[1]; yv1[1] = yv1[2];
            yv1[2] = (xv1[0] + xv1[2]) + coefs[1] * xv1[1] + ( coefs[2] * yv1[0]) + (coefs[3] * yv1[1]);
            pOutput[i] = yv1[2];

            xv2[0] = xv2[1]; xv2[1] = xv2[2];
            xv2[2] = pIn[i+1] / GAIN;
            yv2[0] = yv2[1]; yv2[1] = yv2[2];
            yv2[2] = (xv2[0] + xv2[2]) + coefs[1] * xv2[1] + ( coefs[2] * yv2[0]) + (coefs[3] * yv2[1]);
            pOutput[i+1] = yv2[2];
        }
        else
        {
            // Fourth order
            xv1[0] = xv1[1]; xv1[1] = xv1[2]; xv1[2] = xv1[3]; xv1[3] = xv1[4];
            xv1[4] = pIn[i] / GAIN;
            yv1[0] = yv1[1]; yv1[1] = yv1[2]; yv1[2] = yv1[3]; yv1[3] = yv1[4];
            yv1[4] =   (xv1[0] + xv1[4]) + coefs[1]*(xv1[1]+xv1[3]) + coefs[2] * xv1[2]
                     + ( coefs[3] * yv1[0]) + (  coefs[4] * yv1[1])
                     + ( coefs[5] * yv1[2]) + (  coefs[6] * yv1[3]);
            pOutput[i] = yv1[4];

            xv2[0] = xv2[1]; xv2[1] = xv2[2]; xv2[2] = xv2[3]; xv2[3] = xv2[4];
            xv2[4] = pIn[i+1] / GAIN;
            yv2[0] = yv2[1]; yv2[1] = yv2[2]; yv2[2] = yv2[3]; yv2[3] = yv2[4];
            yv2[4] =   (xv2[0] + xv2[4]) + coefs[1]*(xv2[1]+xv2[3]) + coefs[2] * xv2[2]
                     + ( coefs[3] * yv2[0]) + (  coefs[4] * yv2[1])
                     + ( coefs[5] * yv2[2]) + (  coefs[6] * yv2[3]);
            pOutput[i+1] = yv2[4];
        }
    }

// Check for denormals
    for (i=0; i<=order; ++i)
    {
        xv1[i] = zap_denormal(xv1[i]);
        yv1[i] = zap_denormal(yv1[i]);
        xv2[i] = zap_denormal(xv2[i]);
        yv2[i] = zap_denormal(yv2[i]);
    }
}

