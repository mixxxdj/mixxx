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

#include "enginebufferscale.h"

/**
  *@author Tue & Ken Haste Andersen
  */

class EngineBufferScaleLinear : public EngineBufferScale  {
public: 
    EngineBufferScaleLinear(ReaderExtractWave *wave);
    ~EngineBufferScaleLinear();
    void setRate(double rate);
    CSAMPLE *scale(double playpos, int buf_size);
private:
    /** Holds playback direction */
    bool backwards;
    /** Pointer to ReaderExtractWave buffer */
    CSAMPLE *wavebuffer;
};

#endif
