/***************************************************************************
                          enginebufferscalesrc.h  -  description
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

#ifndef ENGINEBUFFERSCALESRC_H
#define ENGINEBUFFERSCALESRC_H

#include "enginebufferscale.h"
#include <samplerate.h>

/**
  * Performs scaling of audio based on the Secret Rabbit Code (SRC) library.
  *@author Tue & Ken Haste Andersen
  */

class EngineBufferScaleSRC : public EngineBufferScale  {
public: 
    EngineBufferScaleSRC(ReaderExtractWave *wave);
    ~EngineBufferScaleSRC();
    void setQuality(int);
    CSAMPLE *scale(double playpos, int buf_size);
    double setRate(double rate);
private:
    SRC_STATE *converter;
    SRC_DATA *data;
    /** Holds the playback direction */
    bool backwards;
    /** Buffer used to reverse output from SRC library when playback direction is backwards */
    CSAMPLE *buffer_back;
};

#endif
