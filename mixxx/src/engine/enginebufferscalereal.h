/***************************************************************************
                          enginebufferscalereal.h  -  description
                             -------------------
    begin                : Fri Feb 25 2005
    copyright            : (C) 2005 by Tue Haste Andersen
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

#ifndef ENGINEBUFFERSCALEREAL_H
#define ENGINEBUFFERSCALEREAL_H

#include "enginebufferscale.h"

/**
  *@author Tue Haste Andersen
  */

/** Fragment length in seconds */
const float kfRealSearchFragmentLength = 1.;

class ControlObject;
  
class EngineBufferScaleReal : public EngineBufferScale  
{
public:
    EngineBufferScaleReal();
    ~EngineBufferScaleReal();
    double setTempo(double dTempo);
    /** Set base rate */
    void setBaseRate(double dBaseRate);
    void clear();
    CSAMPLE* scale(double playpos, int buf_size, 
                   CSAMPLE* pBase, int iBaseLength);

private:
    ControlObject *m_pControlObjectSampleRate;
    /** Buffer to hold fragment */
    float *m_pFragmentBuffer;
    int m_iFragmentPlaypos;
    int m_iFragmentLength;
    
    /** Holds playback direction */
    bool m_bBackwards;
};

#endif
