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
    float rate_add_new = 2.*m_dBaseRate;
    float rate_add_old = 2.*m_fOldBaseRate; //Smoothly interpolate to new playback rate
    float rate_add = rate_add_new; 
    
    m_fOldBaseRate = m_dBaseRate;           //Update the old base rate because we only need to
                                            //interpolate/ramp up the pitch changes once.
    
    // Determine position in read_buffer to start from
    new_playpos = playpos;

    long unscaled_samples_needed = buf_size + (long)(floor((float)buf_size * ((float)fabs(m_dBaseRate) - 1.0)));

    unscaled_samples_needed = long(ceil(fabs(buf_size * m_dBaseRate)));
    //unscaled_samples_needed = buf_size + floor(buf_size * (fabs(m_dBaseRate) - 1.0f));
    
    if (!even(unscaled_samples_needed))
        unscaled_samples_needed++;
    
    Q_ASSERT(unscaled_samples_needed >= 0);
    Q_ASSERT(unscaled_samples_needed != 0);
    
    int buffer_size = 0;
    double buffer_index= 0;

    // CSAMPLE previous_l = 0;
    // CSAMPLE previous_r = 0;
        
    for (int i = 0; i < buf_size;) {
        long prev = floor(buffer_index);
        if (!even(prev)) prev--;
        long next = prev + 2;

        Q_ASSERT(prev >= 0);
        Q_ASSERT(next >= 0);
        
        /* Fetch enough audio into the internal buffer to access the next
         * sample
         */
        if (next > buffer_size) {
            Q_ASSERT(unscaled_samples_needed > 0);
            int samples_to_read = math_min(kiLinearScaleReadAheadLength,
                                           unscaled_samples_needed);
            buffer_size = m_pReadAheadManager->getNextSamples(m_dBaseRate,
                                                              buffer_int,
                                                              samples_to_read);
            unscaled_samples_needed -= buffer_size;
            buffer_index = 0;
            continue;
        }

        //Smooth any changes in the playback rate over RATE_LERP_LENGTH samples. This
        //prevvents the change from being discontinuous and helps improve sound
        //quality.
        if (i < RATE_LERP_LENGTH) {
            rate_add = (rate_add_new-rate_add_old)/RATE_LERP_LENGTH*i + rate_add_old;
        }
        else {
            rate_add = rate_add_new;
        }
        rate_add = rate_add_new;

        CSAMPLE frac = new_playpos - floor(new_playpos);
            
        //Perform linear interpolation
        buffer[i] = buffer_int[prev] + frac*(buffer_int[next] - buffer_int[prev]);
        buffer[i+1] = buffer_int[prev+1] + frac*(buffer_int[next+1] - buffer_int[prev+1]);
        i += 2;

        // buffer[i] = previous_l + frac*(buffer_int[next] - previous_l);
        // buffer[i+1] = previous_r + frac*(buffer_int[next+1] - previous_r);
        // previous_l = buffer_int[next];
        // previous_r = buffer_int[next+1];

        new_playpos += rate_add;
        buffer_index += fabs(rate_add);
    }
    
    Q_ASSERT(unscaled_samples_needed == 0);
    
    return buffer;
}
