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

EngineBufferScaleLinear::EngineBufferScaleLinear(ReaderExtractWave *wave) : EngineBufferScale(wave)
{
}

EngineBufferScaleLinear::~EngineBufferScaleLinear()
{
}

double EngineBufferScaleLinear::setRate(double _rate)
{
    rate = _rate;

    // Determine playback direction
    if (rate<0.)
        backwards = true;
    else
        backwards = false;

    return rate;
}

CSAMPLE *EngineBufferScaleLinear::scale(double playpos, int buf_size)
{
    double rate_add = 2.*rate;

    // Determine position in read_buffer to start from
    new_playpos = playpos;

    // Prepare buffer
    if (backwards)
    {
        for (int i=0; i<buf_size; i+=2)
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
        int i;
        for (i=0; i<buf_size; i+=2)
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

