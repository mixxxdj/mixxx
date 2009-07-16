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

EngineBufferScaleLinear::EngineBufferScaleLinear() : EngineBufferScale()
{
    m_dBaseRate = 0.0f;
    m_dTempo = 0.0f;
    m_fOldTempo = 1.0f;
    m_fOldBaseRate = 1.0f;
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

    // Prepare buffer
    if (m_bBackwards)
    {
        for (unsigned long i=0; i<buf_size; i+=2)
        {
            long prev = (long)(floor(new_playpos)+READBUFFERSIZE)%READBUFFERSIZE;
            if (!even(prev)) prev--;
            long next = (prev-2+READBUFFERSIZE)%READBUFFERSIZE;

            //Smooth any changes in the playback rate over RATE_LERP_LENGTH samples. This
            //prevvents the change from being discontinuous and helps improve sound
            //quality.
            if (i < RATE_LERP_LENGTH)
            {
                rate_add = (rate_add_new-rate_add_old)/RATE_LERP_LENGTH*i + rate_add_old;
            }
            else
                rate_add = rate_add_new;
            
            CSAMPLE frac = new_playpos-floor(new_playpos);
            
            //Hermite interpolation, see the comments in the "else" block below.
            //buffer[i  ] = hermite4(frac, pBase[math_min(prev-2, 0)], pBase[prev], pBase[next], pBase[(next+2)%READBUFFERSIZE]); 
            //buffer[i+1] = hermite4(frac, pBase[math_min(prev-1, 1)], pBase[prev+1], pBase[(next+1)%READBUFFERSIZE], pBase[(next+3)%READBUFFERSIZE]); 
            
            //Perform linear interpolation
            buffer[i  ] = pBase[prev  ] + frac*(pBase[next  ]-pBase[prev  ]);
            buffer[i+1] = pBase[prev+1] + frac*(pBase[next+1]-pBase[prev+1]);

            new_playpos += rate_add;
        }
    }
    else
    {
        for (unsigned long i=0; i<buf_size; i+=2)
        {
            long prev = (long)floor(new_playpos)%READBUFFERSIZE;
            if (!even(prev)) prev--;
            long next = (prev+2)%READBUFFERSIZE;
          
            //Smooth any changes in the playback rate over RATE_LERP_LENGTH samples. This
            //prevents the change from being discontinuous and helps improve sound
            //quality.
            if (i < RATE_LERP_LENGTH)
            {
                rate_add = (rate_add_new-rate_add_old)/RATE_LERP_LENGTH*i + rate_add_old;
            }
            else
                rate_add = rate_add_new;

            CSAMPLE frac = (new_playpos - floor(new_playpos))/2;
            
            /* //Hermite interpolation - experimented with this to see if there was any noticeable increase
             *                           in sound quality, but I couldn't hear any (the literature generally
             *                           agrees that that is the case too) - Albert 04/27/08
            buffer[i  ] = hermite4(frac, pBase[math_min(prev-2, 0)], 
                                   pBase[prev], pBase[next], pBase[(next+2)%READBUFFERSIZE]); 
            buffer[i+1] = hermite4(frac, pBase[math_min(prev-1, 1)], 
                                   pBase[prev+1], pBase[(next+1)%READBUFFERSIZE], pBase[(next+3)%READBUFFERSIZE]); 
            */
            
            //Perform linear interpolation
            buffer[i  ] = pBase[prev  ] + frac*(pBase[next  ]-pBase[prev  ]);
            buffer[i+1] = pBase[prev+1] + frac*(pBase[next+1]-pBase[prev+1]);

            new_playpos += rate_add;
        }
    }

    return buffer;
}

