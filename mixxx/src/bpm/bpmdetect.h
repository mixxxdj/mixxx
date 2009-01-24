/***************************************************************************
bpmdetect.h  -  adaption of the soundtouch bpm detection code
-------------------
begin                : Sat, Aug 4., 2007
copyright            : (C) 2007 by Micah Lee
email                : snipexv@gmail.com
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

////////////////////////////////////////////////////////////////////////////////
///
/// Beats-per-minute (BPM) detection routine.
///
/// The beat detection algorithm works as follows:
/// - Use function 'inputSamples' to input a chunks of samples to the class for
///   analysis. It's a good idea to enter a large sound file or stream in smallish
///   chunks of around few kilosamples in order not to extinguish too much RAM memory.
/// - Input sound data is decimated to approx 500 Hz to reduce calculation burden,
///   which is basically ok as low (bass) frequencies mostly determine the beat rate.
///   Simple averaging is used for anti-alias filtering because the resulting signal
///   quality isn't of that high importance.
/// - Decimated sound data is enveloped, i.e. the amplitude shape is detected by
///   taking absolute value that's smoothed by sliding average. Signal levels that
///   are below a couple of times the general RMS amplitude level are cut away to
///   leave only notable peaks there.
/// - Repeating sound patterns (e.g. beats) are detected by calculating short-term 
///   autocorrelation function of the enveloped signal.
/// - After whole sound data file has been analyzed as above, the bpm level is 
///   detected by function 'getBpm' that finds the highest peak of the autocorrelation 
///   function, calculates it's precise location and converts this reading to bpm's.
///
/// Author        : Copyright (c) Olli Parviainen
/// Author e-mail : oparviai 'at' iki.fi
/// SoundTouch WWW: http://www.surina.net/soundtouch
///
////////////////////////////////////////////////////////////////////////////////
//
// Last changed  : $Date: 2006/02/05 16:44:06 $
// File revision : $Revision: 1.5 $
//
// $Id: BPMDetect.h,v 1.5 2006/02/05 16:44:06 Olli Exp $
//
////////////////////////////////////////////////////////////////////////////////
//
// License :
//
//  SoundTouch audio processing library
//  Copyright (c) Olli Parviainen
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _BPMDetect_H_
#define _BPMDetect_H_

#include "STTypes.h"
#include "FIFOSampleBuffer.h"

#ifdef __WIN__
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef long int32_t;
typedef unsigned long uint32_t;
typedef long long int64_t;
typedef unsigned long long uint64_t;
#endif

/// Minimum allowed BPM rate. Used to restrict accepted result above a reasonable limit.
#define MIN_BPM 75

/// Maximum allowed BPM rate. Used to restrict accepted result below a reasonable limit.
#define MAX_BPM 170

/// Class for calculating BPM rate for audio data.
class BpmDetect
{
protected:
    /// Auto-correlation accumulator bins.
    float *xcorr;
    
    /// Amplitude envelope sliding average approximation level accumulator
    float envelopeAccu;

    /// RMS volume sliding average approximation level accumulator
    float RMSVolumeAccu;

    /// Sample average counter.
    int decimateCount;

    /// Sample average accumulator for FIFO-like decimation.
    soundtouch::LONG_SAMPLETYPE decimateSum;

    /// Decimate sound by this coefficient to reach approx. 500 Hz.
    unsigned int decimateBy;

    /// Auto-correlation window length
    unsigned int windowLen;

    /// Number of channels (1 = mono, 2 = stereo)
    unsigned int channels;

    /// sample rate
    unsigned int sampleRate;

	/// Maximum acceptable bpm
	unsigned int maxBpm;

	/// Minumum acceptable bpm
	unsigned int minBpm;

    /// Beginning of auto-correlation window: Autocorrelation isn't being updated for
    /// the first these many correlation bins.
    unsigned int windowStart;
 
    /// FIFO-buffer for decimated processing samples.
    soundtouch::FIFOSampleBuffer *buffer;

    /// Initialize the class for processing.
    void init(int numChannels, unsigned int sampleRate);

    /// Updates auto-correlation function for given number of decimated samples that 
    /// are read from the internal 'buffer' pipe (samples aren't removed from the pipe 
    /// though).
    void updateXCorr(int process_samples      /// How many samples are processed.
                     );

    /// Decimates samples to approx. 500 Hz.
    ///
    /// \return Number of output samples.
    int decimate(soundtouch::SAMPLETYPE *dest,      ///< Destination buffer
                 const soundtouch::SAMPLETYPE *src, ///< Source sample buffer
                 int numsamples                     ///< Number of source samples.
                 );

    /// Calculates amplitude envelope for the buffer of samples.
    /// Result is output to 'samples'.
    void calcEnvelope(soundtouch::SAMPLETYPE *samples,  ///< Pointer to input/output data buffer
                      int numsamples                    ///< Number of samples in buffer
                      );

public:
    /// Constructor.
    /// @note _maxBpm should be at least 2 * _minBpm, otherwise BPM won't always be detected
    BpmDetect(int numChannels,          ///< Number of channels in sample data.
              int sampleRate,           ///< Sample rate in Hz.
              int _minBpm = MIN_BPM,    ///< Minimum acceptable BPM
              int _maxBpm = MAX_BPM     ///< Maximum acceptable BPM
    );

    /// Destructor.
    virtual ~BpmDetect();

    /**
     * Multiplying or dividing BPM by 2 if value is lower than min or greater than max
     * @return corrected BPM (can be greater than max)
     */
    static float correctBPM( float BPM, int min, int max );

    /// Inputs a block of samples for analyzing: Envelopes the samples and then
    /// updates the autocorrelation estimation. When whole song data has been input
    /// in smaller blocks using this function, read the resulting bpm with 'getBpm' 
    /// function. 
    /// 
    /// Notice that data in 'samples' array can be disrupted in processing.
    void inputSamples(soundtouch::SAMPLETYPE *samples,  ///< Pointer to input/working data buffer
                      int numSamples                    ///< Number of samples in buffer
                      );


    /// Analyzes the results and returns the BPM rate. Use this function to read result
    /// after whole song data has been input to the class by consecutive calls of
    /// 'inputSamples' function.
    ///
    /// \return Beats-per-minute rate, or zero if detection failed.
    float getBpm();
};

#endif // _BPMDetect_H_
