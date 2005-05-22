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
    EngineFilterIIR(const double *coefs, int order);
    ~EngineFilterIIR();
    void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize);
protected:
    int order;
	/** Used to avoid denormalization on Intel CPUs */
	float id;
    const double *coefs;
    #define MAXNZEROS 8 
    #define MAXNPOLES 8 
    double xv1[MAXNZEROS+1], yv1[MAXNPOLES+1];
    double xv2[MAXNZEROS+1], yv2[MAXNPOLES+1];
};

//
// Defines filter coefficients for IIR filters:
//

// 8th order lowpass, corner at 600 Hz
static const double bessel_lowpass[13] = {  7.444032197e+08, 
					    8, 28, 56, 70,
					    -0.3800297563, 3.4120798629, 
					    -13.4230504610, 30.2214248640,
					    -42.5938048390, 38.4826057150,
					    -21.7665031930, 7.0472774638};

// 4th order highpass, corner at 4000 Hz:
static const double bessel_highpass[13] = {2.465837728e+00, // gain
					   - 8,+ 28, - 56, + 70,
					   -0.1552424571, 1.5489970216,
					   -6.7821376632,17.0223182510,
					   -26.7923322400,27.0856195480,
					   -17.1796384890, 6.2523870250};

// 8th order bandpass at 600 - 4000Hz:
static const double bessel_bandpass[13] = {1.455078491e+02,
					   0,-4,0,6,
					   -0.1002852333, 1.0213655417,
					   -4.6272090652,  12.1726925480,
					   -20.3120761830, 21.9557125490,
					   -14.9560287020,  5.8458265249};


// 2nd order lowpass, corner 600Hz
static const double bessel_lowpass2[4] = {3.707141512e+02, 2, -0.8282366449, 1.8174466602};

// 2nd order bandpass at 600 - 4000Hz:
static const double bessel_bandpass2[7] = {1.596830813e+01, 0, -2, -0.3374389990, 1.7280392126, -3.4124608099, 3.0203698354};

// 2nd order highpass, corner at 4000 Hz:
static const double bessel_highpass2[4] = {1.451889828e+00, -2, -0.4505643044, 1.3044656722}; 

// 4th order lowpass, corner 600Hz
static const double bessel_lowpass4[7] = {6.943736360e+04, 4, 6, -0.6673458737, 2.9444565421, -4.8814113588, 3.6040702669};

// 4th order highpass, corner 4000Hz
static const double bessel_highpass4[7] = {1.807553687e+00, -4, 6, -0.2898387148, 1.5497144728, -3.1422295239, 2.8699599032};

#endif
