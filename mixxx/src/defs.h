/***************************************************************************
                          defs.h  -  description
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

#ifndef DEFS_H
#define DEFS_H

#define VERSION "0.9.2"

#include <math.h>
#include <iostream>
#include <stdlib.h>

typedef short int SAMPLE;       // Data type used in output buffer. S16_LE works on SB/ALSA.
//const int SAMPLE_SIZE = 2;      // Number of bytes used to store 1 sample
typedef float CSAMPLE;          // CSAMPLE defines the CSAMPLE type used for
                                // intermidiate calculations
typedef CSAMPLE FLOAT_TYPE;       // Float type, used for non sample data
const int BUFFER_SIZE  = 2048;  // Buffer size used both for input and output as default
                                            
/** size of the chunk that's read in every read. */
const unsigned int READCHUNKSIZE = 23040;
/** Number of readchunks. Should minimum be set to 5. In theory this should minimum be four, but
  * since it is possible that the sound sources return more samples than request, 5 is on the
  * safe side */
const int READCHUNK_NO = 8;
const unsigned int READBUFFERSIZE = READCHUNKSIZE*READCHUNK_NO;
/** Window size used in EnginePreProcess */
const int WINDOWSIZE = 2304; //4608; //512;
/** Step size used in block based processing (EnginePreProcess and others) */
const int STEPSIZE = 256; //WINDOWSIZE/2;

/** Maximum buffer length to each EObject::process call */
const int MAX_BUFFER_LEN = 400000;

// Various fixed constants
// static CSAMPLE pi     = acos(-1.0); // Conflicts with macx headers
static CSAMPLE two_pi = (2.*acos(-1.0));

// Defs for the ports and their midi control values:
const short ADC0 = 0x00;
const short ADC1 = 0x08;
const short ADC2 = 0x10;
const short ADC3 = 24  ;
const short ADC4 = 32  ;
const short ADC5 = 40  ;
const short ADC6 = 0x30;
const short ADC7 = 0x38;
const short PORT_B = 1;
const short PORT_D = 3;

// Ensure that CSAMPLE x stays above the intel cpu denormalization range,
// otherwise sets x equal to 0.
inline CSAMPLE zap_denormal(CSAMPLE x)
{
    CSAMPLE absx = fabs(x);
    return (absx > 1e-15 && absx < 1e15) ? x : 0.;
}

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#endif


