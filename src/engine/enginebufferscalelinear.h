/***************************************************************************
                          enginebufferscalelinear.h  -  description
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

#ifndef ENGINEBUFFERSCALELINEAR_H
#define ENGINEBUFFERSCALELINEAR_H

#include "engine/enginebufferscale.h"
#include "engine/readaheadmanager.h"

/**
  *@author Tue & Ken Haste Andersen
  */


/** Number of samples to read ahead */
const int kiLinearScaleReadAheadLength = 10240;


class EngineBufferScaleLinear : public EngineBufferScale  {
  public:
    EngineBufferScaleLinear(ReadAheadManager *pReadAheadManager);
    virtual ~EngineBufferScaleLinear();

    CSAMPLE* getScaled(unsigned long buf_size);
    void clear();

    virtual void setScaleParameters(double base_rate,
                                    double* pTempoRatio,
                                    double* pPitchRatio);

  private:
    CSAMPLE* do_scale(CSAMPLE* buf, unsigned long buf_size,
                      int *samples_read);

    /** Holds playback direction */
    bool m_bBackwards;
    bool m_bClear;
    double m_dRate;
    double m_dOldRate;

    // Buffer for handling calls to ReadAheadManager
    CSAMPLE* m_bufferInt;
    int m_bufferIntSize;
    CSAMPLE m_floorSampleOld[2];
    // The read-ahead manager that we use to fetch samples
    ReadAheadManager* m_pReadAheadManager;
    double m_dCurrentFrame;
    double m_dNextFrame;
};

#endif
