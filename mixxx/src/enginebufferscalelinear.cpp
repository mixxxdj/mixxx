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

#include "enginebufferscalelinear.h"
#include "mathstuff.h"
#include "readerextractwave.h"

EngineBufferScaleLinear::EngineBufferScaleLinear(ReaderExtractWave * wave) : EngineBufferScale(wave)
{
}

EngineBufferScaleLinear::~EngineBufferScaleLinear()
{
}

double EngineBufferScaleLinear::setTempo(double _tempo)
{
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
    m_dBaseRate = dBaseRate*m_dTempo;
}

void EngineBufferScaleLinear::clear()
{
    m_bClear = true;
}

CSAMPLE * EngineBufferScaleLinear::scale(double playpos, unsigned long buf_size, float * pBase, unsigned long iBaseLength)
{
    if (!pBase)
    {
        pBase = wavebuffer;				//The "base" buffer is really 
        								//the EngineBuffer's circular 
        								//audio buffer.
        iBaseLength = READBUFFERSIZE;	//Length of the base buffer
    }
    
    float rate_add = 2.*m_dBaseRate;

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

            CSAMPLE frac = new_playpos-floor(new_playpos);
            buffer[i  ] = wavebuffer[prev  ] + frac*(wavebuffer[next  ]-wavebuffer[prev  ]);
            buffer[i+1] = wavebuffer[prev+1] + frac*(wavebuffer[next+1]-wavebuffer[prev+1]);

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

            CSAMPLE frac = new_playpos - floor(new_playpos);
            buffer[i  ] = wavebuffer[prev  ] + frac*(wavebuffer[next  ]-wavebuffer[prev  ]);
            buffer[i+1] = wavebuffer[prev+1] + frac*(wavebuffer[next+1]-wavebuffer[prev+1]);

            new_playpos += rate_add;
        }
    }

    return buffer;
}

