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
#include <qobject.h>

#define MAX_SEEK_SPEED 4.0f
#define MIN_SEEK_SPEED 0.0001f

class ReaderExtractWave;

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
    double getNewPlaypos();
    /** Called from EngineBuffer when seeking, to ensure the buffers are flushed */
    virtual void clear() = 0;
    /** Scale buffer */
    virtual CSAMPLE *scale(double playpos, 
                           unsigned long buf_size,
                           CSAMPLE* pBase,
                           unsigned long iBaseLength) = 0;
    
protected:
    /** Tempo and base rate */
    double m_dTempo, m_dBaseRate;
    /** Pointer to internal buffer */
    CSAMPLE *buffer;
    /** New playpos after call to scale */
    double new_playpos;
};

#endif
