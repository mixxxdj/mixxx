/***************************************************************************

                          enginebufferscalest.h  -  description
                             -------------------

    begin                : November 2004
    copyright            : (C) 2004 by Tue Haste Andersen
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

/**
  *@author Tue Haste Andersen
*/

#ifndef ENGINEBUFFERSCALEST_H
#define ENGINEBUFFERSCALEST_H

#include "engine/enginebufferscale.h"

// Number of samples to read ahead. Setting this too high (10000) causes
// stuttering.
const int kiSoundTouchReadAheadLength = 1000;
class ReadAheadManager;

namespace soundtouch {
class SoundTouch;
}  // namespace soundtouch

// Uses libsoundtouch to scale audio.
class EngineBufferScaleST : public EngineBufferScale {
    Q_OBJECT
  public:
    EngineBufferScaleST(ReadAheadManager* pReadAheadManager);
    virtual ~EngineBufferScaleST();

    virtual void setScaleParameters(double base_rate,
                                    double* pTempoRatio,
                                    double* pPitchRatio);

    virtual void setSampleRate(int iSampleRate);

    // Scale buffer.
    CSAMPLE* getScaled(unsigned long buf_size);

    // Flush buffer.
    void clear();

  private:
    // Holds the playback direction.
    bool m_bBackwards;

    // Temporary buffer for reading from the RAMAN.
    CSAMPLE* buffer_back;

    // SoundTouch time/pitch scaling lib
    soundtouch::SoundTouch* m_pSoundTouch;

    // The read-ahead manager that we use to fetch samples
    ReadAheadManager* m_pReadAheadManager;
};

#endif
