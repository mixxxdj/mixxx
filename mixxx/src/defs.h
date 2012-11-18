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

#define MIXXX_PROMO_DIR "promo"

#include <math.h>
#include <iostream>
#include <stdlib.h>

typedef short int SAMPLE;       // Data type used in output buffer. S16_LE works on SB/ALSA.
//const int SAMPLE_SIZE = 2;      // Number of bytes used to store 1 sample
typedef float CSAMPLE;          // CSAMPLE defines the CSAMPLE type used for
                                // intermidiate calculations
typedef CSAMPLE FLOAT_TYPE;       // Float type, used for non sample data

const int OK = 0;               // Just defs to use for returning errors from functions
const int ERR = -1;

const int BUFFER_SIZE  = 2048;  // Buffer size used both for input and output as default

/** size of the chunk that's read in every read. */
const unsigned int READCHUNKSIZE = 20480; //40960;
/** Number of readchunks. Should minimum be set to 5. In theory this should minimum be four, but
  * since it is possible that the sound sources return more samples than request, 5 is on the
  * safe side */
const int READCHUNK_NO = 40;
const unsigned int READBUFFERSIZE = READCHUNKSIZE*READCHUNK_NO;
/** Window size used in ReaderExtract objects */
const int WINDOWSIZE = 2048;
/** Step size used in block based processing (ReaderExtract classes) */
const int STEPSIZE = 1024; //WINDOWSIZE/2; //WINDOWSIZE/STEPSIZE must result in an integer value

/** Maximum buffer length to each EngineObject::process call */
const int MAX_BUFFER_LEN = 160000;

#ifndef PATH_MAX
#ifndef MAX_PATH
// http://msdn.microsoft.com/en-us/library/windows/desktop/aa365247%28v=vs.85%29.aspx
const int MAX_PATH = 260;
#endif
// Use POSIX name for MAX_PATH
enum {
	PATH_MAX = MAX_PATH
};
#endif

// Various fixed constants
// static CSAMPLE pi     = acos(-1.0); // Conflicts with macx headers
// static CSAMPLE two_pi = (2.f*acos(-1.f));
// two_pi has been moved to mathstuff.h clear up the "defs.h:55: warning: ‘two_pi’ defined but not used" it generates for every file including defs.h

// Ensure that CSAMPLE x stays above the intel cpu denormalization range,
// otherwise sets x equal to 0.
inline double zap_denormal(double x)
{
    // fabs too slow on Windows...
    double absx;
    if (x<0)
        absx = -x;
    else
        absx = x;

    return (absx > 1e-15f && absx < 1e15f) ? x : 0.f;
}

#ifndef math_max
#define math_max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef math_min
#define math_min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

// MSVC 2005/2008 needs these
#ifndef fmax
#define fmax math_max
#endif

#ifndef fmin
#define fmin math_min
#endif

#endif

