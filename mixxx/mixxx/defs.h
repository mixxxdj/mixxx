#ifndef DEFS_H
#define DEFS_H

#include <math.h>
#include <iostream>
#include <stdlib.h>

typedef short int SAMPLE;       // Data type used in output buffer. S16_LE works on SB/ALSA.
const int SAMPLE_SIZE = 2;      // Number of bytes used to store 1 sample
typedef double CSAMPLE;          // CSAMPLE defines the CSAMPLE type used for
                                // intermidiate calculations
typedef CSAMPLE FLOAT;            // Float type, used for non sample data
static int SRATE       = 44100; // Sample rate
const int BUFFER_SIZE  = 128;  // Buffer size used both for input and output
const int READAHEAD = 150000;     // how many samples to read ahead.
const int READCHUNKSIZE = 1500000; // size of the chunk that's read in every read.
const int READBUFFERSIZE = 2*READCHUNKSIZE; // size of read buffer.
const int NO_CHANNELS  = 2;     // 2 for stereo

/** Maximum buffer length to each EObject::process call */
const int MAX_BUFFER_LEN = 100000;

// Various fixed constants
static int NYQUIST    = SRATE/2;
static CSAMPLE norm   = (2*acos(-1.0))/SRATE;
static CSAMPLE pi     = acos(-1.0);
static CSAMPLE two_pi = (2*acos(-1.0));

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

#endif
