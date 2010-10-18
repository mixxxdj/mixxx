/***************************************************************************
                          enginebufferscalelinear.cpp  -  description
                            -------------------
    begin                : Mon Apr 14 2003
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
    email                : haste@diku.dk
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QtCore>
#include "enginebufferscalelinear.h"
#include "mathstuff.h"

#define RATE_LERP_LENGTH 200

EngineBufferScaleLinear::EngineBufferScaleLinear(ReadAheadManager *pReadAheadManager) :
    EngineBufferScale(),
    m_pReadAheadManager(pReadAheadManager)
{
    m_dBaseRate = 0.0f;
    m_dTempo = 0.0f;
    m_fOldTempo = 1.0f;
    m_fOldBaseRate = 1.0f;
    m_fPreviousL = 0.0f;
    m_fPreviousR = 0.0f;
    m_scaleRemainder = 0.0f;

    buffer_int = new CSAMPLE[kiLinearScaleReadAheadLength];
}

EngineBufferScaleLinear::~EngineBufferScaleLinear()
{
}

double EngineBufferScaleLinear::setTempo(double _tempo)
{
//    if (m_fOldTempo != m_dTempo)
        m_fOldTempo = m_dTempo; //Save the old tempo when the tempo changes

    m_dTempo = _tempo;

    if (m_dTempo>MAX_SEEK_SPEED)
        m_dTempo = MAX_SEEK_SPEED;
    else if (m_dTempo < -MAX_SEEK_SPEED)
        m_dTempo = -MAX_SEEK_SPEED;

    // Determine playback direction
    if (m_dTempo<0.)
        m_bBackwards = true;
    else
        m_bBackwards = false;

    return m_dTempo;
}

void EngineBufferScaleLinear::setBaseRate(double dBaseRate)
{
//    if (m_fOldBaseRate != m_dBaseRate)
        m_fOldBaseRate = m_dBaseRate; //Save the old baserate when it changes

    m_dBaseRate = dBaseRate*m_dTempo;
}

void EngineBufferScaleLinear::clear()
{
    m_bClear = true;
    m_scaleRemainder = 0.0f;
}


// laurent de soras - punked from musicdsp.org (mad props)
inline float hermite4(float frac_pos, float xm1, float x0, float x1, float x2)
{
    const float c = (x1 - xm1) * 0.5f;
    const float v = x0 - x1;
    const float w = c + v;
    const float a = w + v + (x2 - x0) * 0.5f;
    const float b_neg = w + a;
    return ((((a * frac_pos) - b_neg) * frac_pos + c) * frac_pos + x0);
}

/** Stretch a buffer worth of audio using linear interpolation */
CSAMPLE * EngineBufferScaleLinear::scale(double playpos, unsigned long buf_size,
                                         CSAMPLE* pBase, unsigned long iBaseLength)
{

    long unscaled_samples_needed;
    float rate_add_new = m_dBaseRate;
    float rate_add_old = m_fOldBaseRate; //Smoothly interpolate to new playback rate
    float rate_add = rate_add_new;
    float rate_add_diff = rate_add_new - rate_add_old;
    double rate_add_abs;

    if ( rate_add_diff )
        m_scaleRemainder = 0.0f;

    //Update the old base rate because we only need to
    //interpolate/ramp up the pitch changes once.
    m_fOldBaseRate = m_dBaseRate;

    // Determine position in read_buffer to start from. (This is always 0 with
    // the new EngineBuffer implementation)
    new_playpos = playpos;

    const int iRateLerpLength = math_min(RATE_LERP_LENGTH, buf_size);

    // Guard against buf_size == 0
    if (iRateLerpLength == 0)
        return buffer;

    // Simulate the loop to estimate how many samples we need
    double samples = 0;

    for (int j = 0; j < iRateLerpLength; j+=2)
    {
        rate_add = (rate_add_diff) / iRateLerpLength * j + rate_add_old;
        samples += fabs(rate_add);
    }

    rate_add = rate_add_new;
    rate_add_abs = fabs(rate_add);

    samples += (rate_add_abs * ((buf_size - iRateLerpLength)/2));
    unscaled_samples_needed = ceil(samples);


    if ( samples != unscaled_samples_needed)
        m_scaleRemainder += (double)unscaled_samples_needed - samples;

    bool carry_remainder = FALSE;
    if ((m_scaleRemainder > 1) || (m_scaleRemainder < 1))
    {
        long rem = (long)floor(m_scaleRemainder);

        // Be very defensive about equating the remainder
        // back into unscaled_samples_needed
	if ((unscaled_samples_needed - rem) >= 1)
	{
            carry_remainder = TRUE;
            m_scaleRemainder -= rem;
            unscaled_samples_needed -= rem;
        }

    }

    // Multiply by 2 because it is predicting mono rates, while we want a stereo
    // number of samples.
    unscaled_samples_needed *= 2;

    Q_ASSERT(unscaled_samples_needed >= 0);
    Q_ASSERT(unscaled_samples_needed != 0);

    int buffer_size = 0;
    double buffer_index = 0;

    long current_sample = 0;
    long prev_sample = 0;
    bool last_read_failed = false;

    // Use new_playpos to count the new samples we touch.
    new_playpos = 0;

    int i = 0;
    int screwups = 0;
    while(i < buf_size)
    {
        prev_sample = current_sample;

        current_sample = floor(buffer_index) * 2;
        if (!even(current_sample))
            current_sample++;

        Q_ASSERT(current_sample % 2 == 0);
        Q_ASSERT(current_sample >= 0);
        
        //This code is so messed up. These ASSERTs should be enabled, but they actually
        //fire because of bug(s). 
        //Q_ASSERT(prev_sample >= 0);
        //Q_ASSERT(prev_sample-1 < kiLinearScaleReadAheadLength); 
        //the prev_sample-1 leaves room for the other sample in the stereo frame
        //Instead, we're going to workaround the bug by just clamping prev_sample
        //to make sure it stays in bounds:
        prev_sample = math_min(kiLinearScaleReadAheadLength, prev_sample);
        prev_sample = math_max(0, prev_sample);


        if (prev_sample != current_sample) {
            m_fPreviousL = buffer_int[prev_sample];
            m_fPreviousR = buffer_int[prev_sample+1];
        }

        if (current_sample+1 >= buffer_size) {
            //Q_ASSERT(unscaled_samples_needed > 0);
            if (unscaled_samples_needed == 0) {
                unscaled_samples_needed = 2;
                screwups++;
            }

            int samples_to_read = math_min(kiLinearScaleReadAheadLength,
                                           unscaled_samples_needed);

            buffer_size = m_pReadAheadManager
                                ->getNextSamples(m_dBaseRate,buffer_int,
                                                 samples_to_read);

            if (m_dBaseRate > 0)
                new_playpos += buffer_size;
            else if (m_dBaseRate < 0)
                new_playpos -= buffer_size;


            if (buffer_size == 0 && last_read_failed) {
                break;
            }
            last_read_failed = buffer_size == 0;

            unscaled_samples_needed -= buffer_size;
            buffer_index = buffer_index - floor(buffer_index);

            continue;
        }

        //Smooth any changes in the playback rate over iRateLerpLength
        //samples. This prevents the change from being discontinuous and helps
        //improve sound quality.
        if (i < iRateLerpLength) {
            rate_add = (rate_add_diff) / iRateLerpLength * i + rate_add_old;
        }
        else {
            rate_add = rate_add_new;
        }

        CSAMPLE frac = buffer_index - floor(buffer_index);

        //Perform linear interpolation
        buffer[i] = m_fPreviousL + frac * (buffer_int[current_sample] - m_fPreviousL);
        buffer[i+1] = m_fPreviousR + frac * (buffer_int[current_sample+1] - m_fPreviousR);

        if (i < iRateLerpLength)
            buffer_index += fabs(rate_add);
        else
            buffer_index += rate_add_abs;

        i+=2;
    }

    if ( carry_remainder )
    {
        m_fPreviousL = buffer_int[buffer_size-2];
        m_fPreviousR = buffer_int[buffer_size-1];
    }

    // If we broke out of the loop, zero the remaining samples
    // TODO(XXX) memset
    for (; i < buf_size; i += 2) {
        buffer[i] = 0.0f;
        buffer[i+1] = 0.0f;
    }

    // It's possible that we will exit this function without having satisfied
    // this requirement. We may be trying to read past the end of the file.
    //Q_ASSERT(unscaled_samples_needed == 0);

    return buffer;
}
