#ifndef DEFS_H
#define DEFS_H

#define VERSION "0.2.1"

#include <math.h>
#include <iostream>
#include <stdlib.h>

typedef short int SAMPLE;       // Data type used in output buffer. S16_LE works on SB/ALSA.
const int SAMPLE_SIZE = 2;      // Number of bytes used to store 1 sample
typedef float CSAMPLE;          // CSAMPLE defines the CSAMPLE type used for
                                // intermidiate calculations
typedef CSAMPLE FLOAT_TYPE;       // Float type, used for non sample data
static int SRATE       = 44100; // Sample rate
const int BUFFER_SIZE  = 64;  // Buffer size used both for input and output
const int READAHEAD = 200000;     // how many samples to read ahead.
const int READCHUNKSIZE = 1500000; // size of the chunk that's read in every read.
const int READBUFFERSIZE = 3*READCHUNKSIZE; // size of read buffer must be at least three
                                            // times READCHUNKSIZE (one extra is needed in case of
                                            // "stop" is pressed.

const int NO_CHANNELS  = 2;     // 2 for stereo

/** Maximum buffer length to each EObject::process call */
const int MAX_BUFFER_LEN = 100000;

// Various fixed constants
static int NYQUIST    = SRATE/2;
static CSAMPLE norm   = (2.*acos(-1.0))/SRATE;
static CSAMPLE pi     = acos(-1.0);
static CSAMPLE two_pi = (2.*acos(-1.0));
static CSAMPLE ln_2   = 0.69314718055994530942;

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


