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

    // Sets the scaling parameters.
    // * The rate adjustment describes the rate change in percentage of original
    //   rate. For example, a rate adjustment of 1.0 is no change.
    // * The tempo adjustment describes the tempo change in percentage of
    //   original tempo. For example, a tempo adjustment of 1.0 is no change.
    // * The pitch adjustment describes the pitch adjustment in percentage of
    //   octaves. For example, a pitch adjustment of 1.0 is no change.
    //
    // If parameter settings are outside of acceptable limits, each setting will
    // be set to the value it was clamped to.
    virtual void setScaleParameters(double* rate_adjust,
                                    double* tempo_adjust,
                                    double* pitch_adjust) {
        m_dRateAdjust = *rate_adjust;
        m_dTempoAdjust = *tempo_adjust;
        m_dPitchAdjust = *pitch_adjust;
    }

    /** Get new playpos after call to scale() */
    double getSamplesRead();
    /** Called from EngineBuffer when seeking, to ensure the buffers are flushed */
    virtual void clear() = 0;
    /** Scale buffer */
    virtual CSAMPLE *getScaled(unsigned long buf_size) = 0;

protected:
    double m_dRateAdjust, m_dTempoAdjust, m_dPitchAdjust;
    /** Pointer to internal buffer */
    CSAMPLE *m_buffer;
    /** New playpos after call to scale */
    double m_samplesRead;
};

#endif
