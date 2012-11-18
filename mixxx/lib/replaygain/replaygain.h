/*
 *  ReplayGainAnalysis - analyzes input samples and give the recommended dB change
 *  Copyright (C) 2001 David Robinson and Glen Sawyer
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  concept and filter values by David Robinson (David@Robinson.org)
 *    -- blame him if you think the idea is flawed
 *  coding by Glen Sawyer (glensawyer@hotmail.com) 442 N 700 E, Provo, UT 84606 USA
 *    -- blame him if you think this runs too slowly, or the coding is otherwise flawed
 *  minor cosmetic tweaks to integrate with FLAC by Josh Coalson
 *
 *  For an explanation of the concepts and the basic algorithms involved, go to:
 *    http://www.replaygain.org/

 */


#ifndef REPLAYGAIN_H_
#define REPLAYGAIN_H_

#include <stddef.h>

#define GAIN_NOT_ENOUGH_SAMPLES  -24601

#define YULE_ORDER         10
#define BUTTER_ORDER        2
#define RMS_PERCENTILE      0.95        /* percentile which is louder than the proposed level */
#define MAX_SAMP_FREQ   48000.          /* maximum allowed sample frequency [Hz] */
#define RMS_WINDOW_TIME     0.050       /* Time slice size [s] */
#define STEPS_per_dB      100.          /* Table entries per dB */
#define MAX_dB            120.          /* Table entries for 0...MAX_dB (normal max. values are 70...80 dB) */

#define MAX_ORDER               (BUTTER_ORDER > YULE_ORDER ? BUTTER_ORDER : YULE_ORDER)

/* [JEC] the following was originally #defined as:
 *   (size_t) (MAX_SAMP_FREQ * RMS_WINDOW_TIME)
 * but that seemed to fail to take into account the ceil() part of the
 * sampleWindow calculation in ResetSampleFrequency(), and was causing
 * buffer overflows for 48kHz analysis, hence the +1.
 */
#ifndef __sun
 #define MAX_SAMPLES_PER_WINDOW  (size_t) (MAX_SAMP_FREQ * RMS_WINDOW_TIME + 1.)   /* max. Samples per Time slice */
#else
 /* [JEC] Solaris Forte compiler doesn't like float calc in array indices */
 #define MAX_SAMPLES_PER_WINDOW  (size_t) (2401)
#endif
#define PINK_REF                64.82 /* 298640883795 */                          /* calibration value */

typedef unsigned short  Uint16_t;
typedef signed short    Int16_t;
typedef unsigned int    Uint32_t;
typedef signed int      Int32_t;

class ReplayGain {
  public:
    ReplayGain ();
    virtual ~ReplayGain();

    bool initialise(long samplefreq, size_t channels);
    bool process(const float* left_samples, const float* right_samples, size_t blockSize);
    float end();

  private:
    void filterYule (const float* input, float* output, size_t nSamples);
    void filterButter (const float* input, float* output, size_t nSamples);
    bool ResetSampleFrequency ( long samplefreq );
    float analyzeResult ( unsigned int* Array, size_t len );

    int             num_channels;
    float           linprebuf [MAX_ORDER * 2];
    float*          linpre;                                          // left input samples, with pre-buffer
    float           lstepbuf  [MAX_SAMPLES_PER_WINDOW + MAX_ORDER];
    float*          lstep;                                           // left "first step" (i.e. post first filter) samples
    float           loutbuf   [MAX_SAMPLES_PER_WINDOW + MAX_ORDER];
    float*          lout;                                            // left "out" (i.e. post second filter) samples
    float           rinprebuf [MAX_ORDER * 2];
    float*          rinpre;                                          // right input samples ...
    float           rstepbuf  [MAX_SAMPLES_PER_WINDOW + MAX_ORDER];
    float*          rstep;
    float           routbuf   [MAX_SAMPLES_PER_WINDOW + MAX_ORDER];
    float*          rout;
    unsigned int            sampleWindow;                                    // number of samples required to reach number of milliseconds required for RMS window
    unsigned long            totsamp;
    double          lsum;
    double          rsum;
    int             freqindex;
#ifndef __sun
    Uint32_t  A [(size_t)(STEPS_per_dB * MAX_dB)];
    /*static Uint32_t  B [(size_t)(STEPS_per_dB * MAX_dB)];*/
#else
    /* [JEC] Solaris Forte compiler doesn't like float calc in array indices */
    Uint32_t  A [12000];
    /*static Uint32_t  B [12000];*/
#endif
};

#endif /* REPLAYGAIN_H_ */
