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
    /** Set fast mode on or off. In fast mode the lowest quality is selected.
      * When set to off, ie. normal mode, the previous selected quality is used */
    void setFastMode(bool bMode);
    CSAMPLE *scale(double playpos, int buf_size, float *pBase=0, int iBaseLength=0);
    double setRate(double rate);
private:
    void setQuality(int);
    /** Pointer to converter objects of three different qualities, and a pointer to
      * the current active object */
    SRC_STATE *converter2, *converter3, *converter4, *converterActive;
    /** Current quality */
    int m_iQuality;
    SRC_DATA *data;
    /** Holds the playback direction */
    bool backwards;
    /** Buffer used to reverse output from SRC library when playback direction is backwards */
    CSAMPLE *buffer_back;
};

#endif
