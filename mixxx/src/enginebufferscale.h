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

class ReaderExtractWave;

/**
  *@author Tue & Ken Haste Andersen
  */

class EngineBufferScale {
public:
    EngineBufferScale(ReaderExtractWave *_wave);
    virtual ~EngineBufferScale();
    virtual void setPitchIndpTimeStretch(bool b);
    virtual void setFastMode(bool bMode);
    /** Set scaling rate */
    virtual double setRate(double _rate) = 0;
    /** Get new playpos after call to scale() */
    double getNewPlaypos();
    /** Called from EngineBuffer when seeking, to ensure the buffers are flushed */
    virtual void clear();
    virtual CSAMPLE *scale(double playpos, int buf_size, float *pBase=0, int iBaseLength=0) = 0;
protected:
    /** Pointer to ReaderExtractWave object */
    ReaderExtractWave *wave;
    /** Pointer to ReaderExtractWave buffer */
    CSAMPLE *wavebuffer;
    /** Rate */
    double rate;
    /** Pointer to internal buffer */
    CSAMPLE *buffer;
    /** New playpos after call to scale */
    double new_playpos;

};

#endif
