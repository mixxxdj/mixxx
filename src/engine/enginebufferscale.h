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

#include <QObject>

#include "util/types.h"

// MAX_SEEK_SPEED needs to be good and high to allow room for the very high
//  instantaneous velocities of advanced scratching (Uzi) and spin-backs.
//  (Yes, I can actually spin the SCS.1d faster than 15x nominal.
//  Why do we even have this parameter? -- Sean)
#define MAX_SEEK_SPEED 100.0
#define MIN_SEEK_SPEED 0.010
// I'll hurt you if you change MIN_SEEK_SPEED. SoundTouch freaks out and
// just gives us stuttering if you set the speed to be lower than this.
// This took me ages to figure out.
// -- Albert July 17, 2010.

/**
  *@author Tue & Ken Haste Andersen
  */

class EngineBufferScale : public QObject {
    Q_OBJECT
  public:
    EngineBufferScale();
    virtual ~EngineBufferScale();

    // Sets the scaling parameters.
    // * The base rate (ratio of track sample rate to output sample rate).
    // * The tempoRatio describes the tempo change in fraction of
    //   original tempo. Put another way, it is the ratio of track seconds to
    //   real second. For example, a tempo of 1.0 is no change. A
    //   tempo of 2 is a 2x speedup (2 track seconds pass for every 1
    //   real second).
    // * The pitchRatio describes the pitch adjustment in fraction of
    //   the original pitch. For example, a pitch adjustment of 1.0 is no change and a
    //   pitch adjustment of 2.0 is a full octave shift up.
    //
    // If parameter settings are outside of acceptable limits, each setting will
    // be set to the value it was clamped to.
    virtual void setScaleParameters(double base_rate,
                                    double* pTempoRatio,
                                    double* pPitchRatio) {
        m_dBaseRate = base_rate;
        m_dTempoRatio = *pTempoRatio;
        m_dPitchRatio = *pPitchRatio;
    }

    // Set the desired output sample rate.
    virtual void setSampleRate(int iSampleRate) {
        m_iSampleRate = iSampleRate;
    }

    /** Get new playpos after call to scale() */
    double getSamplesRead();
    /** Called from EngineBuffer when seeking, to ensure the buffers are flushed */
    virtual void clear() = 0;
    /** Scale buffer */
    virtual CSAMPLE* getScaled(unsigned long buf_size) = 0;

  protected:
    int m_iSampleRate;
    double m_dBaseRate;
    bool m_bSpeedAffectsPitch;
    double m_dTempoRatio;
    double m_dPitchRatio;
    /** Pointer to internal buffer */
    CSAMPLE* m_buffer;
    /** New playpos after call to scale */
    double m_samplesRead;
};

#endif
