/***************************************************************************
                          enginefilteriir.h  -  description
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

#ifndef ENGINEFILTERIIR_H
#define ENGINEFILTERIIR_H

#include "engineobject.h"
#include "defs.h"

class EngineFilterIIR : public EngineObject 
{
public:
    EngineFilterIIR(const float *coefs);
    ~EngineFilterIIR();
    void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize);
protected:
    const float *coefs;
    #define NZEROS 8
    #define NPOLES 8
    float xv[NZEROS+1], yv[NPOLES+1];
};

//
// Defines filter coefficients for IIR filters:
//

// corner at 600 Hz
static const float bessel_lowpass[13] = { 7.444032197e+08,
					    8, 28, 56, 70,
					    -0.3800297563, 3.4120798629, 
					    -13.4230504610, 30.2214248640,
					    -42.5938048390, 38.4826057150,
					    -21.7665031930, 7.0472774638};

static const float bessel_highpass_15000[13] = {1.155528189e+02,
						 -8,28,-56,70,
						 -0.0000256948, -0.0000692066,
						 -0.0047711800, 0.0030454662,
						 -0.1122770236, 0.0900031430,
						 -0.6804656031, 0.3249181788};

static const float bessel_highpass_5000[13] = {3.115907019e+00,
						- 8, 28, - 56, 70,
						-0.0938545059, 0.9902863433,
						-4.5923856269,12.2296988290,
						-20.4633626500,22.0401949270,
						-14.9300062810, 5.8192734790};


// corner at 4000 Hz:
static const float bessel_highpass[13] = {2.465837728e+00, // gain
					   - 8,+ 28, - 56, + 70,
					   -0.1552424571, 1.5489970216,
					   -6.7821376632,17.0223182510,
					   -26.7923322400,27.0856195480,
					   -17.1796384890, 6.2523870250};

// 4th order bandpass at 600 - 4000Hz:
static const float bessel_bandpass[13] = {1.455078491e+02,
					   0,-4,0,6,
					   -0.1002852333, 1.0213655417,
					   -4.6272090652,  12.1726925480,
					   -20.3120761830, 21.9557125490,
					   -14.9560287020,  5.8458265249};

#endif
