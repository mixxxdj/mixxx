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
    EngineBufferScaleLinear();
    ~EngineBufferScaleLinear();
    CSAMPLE *scale(double playpos, unsigned long buf_size, 
                   CSAMPLE* pBase, unsigned long iBaseLength);
    void setBaseRate(double dBaseRate);
    double setTempo(double dTempo);
    void clear();

private:
    /** Holds playback direction */
    bool m_bBackwards;
    bool m_bClear;
    float m_fOldTempo;          /** Keep the old tempo around so we can interpolate smoothly
                                    between the old one and the new one to avoid any discontinuities
                                    in the audio when you change the playback rate */
    float m_fOldBaseRate;       /** Same as old tempo, but for the base playback rate */
};

#endif
