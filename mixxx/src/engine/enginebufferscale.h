/***************************************************************************
                          enginebufferscale.h  -  description
                             -------------------
    begin                : Sun Apr 13 2003
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

#ifndef ENGINEBUFFERSCALE_H
#define ENGINEBUFFERSCALE_H

#include "defs.h"
#include <QObject>

// MAX_SEEK_SPEED needs to be good and high to allow room for the very high
//  instantaneous velocities of advanced scratching (Uzi) and spin-backs.
//  (Yes, I can actually spin the SCS.1d faster than 15x nominal.
//  Why do we even have this parameter? -- Sean)
#define MAX_SEEK_SPEED 100.0f
#define MIN_SEEK_SPEED 0.010f
// I'll hurt you if you change MIN_SEEK_SPEED. SoundTouch freaks out and
// just gives us stuttering if you set the speed to be lower than this.
// This took me ages to figure out.
// -- Albert July 17, 2010.

/**
  *@author Tue & Ken Haste Andersen
  */

class EngineBufferScale : public QObject
{
public:
    EngineBufferScale();
    virtual ~EngineBufferScale();

    /** Set base tempo, ie. normal playback speed. */
    virtual void setBaseRate(double dBaseRate) = 0;
    /** Set tempo */
    virtual double setTempo(double dTempo) = 0;
    /** Get new playpos after call to scale() */
    double getSamplesRead();
    /** Called from EngineBuffer when seeking, to ensure the buffers are flushed */
    virtual void clear() = 0;
    /** Scale buffer */
    virtual CSAMPLE *getScaled(unsigned long buf_size) = 0;

protected:
    /** Tempo and base rate */
    double m_dTempo, m_dBaseRate;
    /** Pointer to internal buffer */
    CSAMPLE *m_buffer;
    /** New playpos after call to scale */
    double m_samplesRead;
};

#endif
