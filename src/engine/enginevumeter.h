/***************************************************************************
                          enginevumeter.h  -  description
                             -------------------
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ENGINEVUMETER_H
#define ENGINEVUMETER_H

#include "engine/engineobject.h"

// Rate at which the vumeter is updated (using a sample rate of 44100 Hz):
#define UPDATE_RATE 20

//SMOOTHING FACTORS
//Must be from 0-1 the lower the factor, the more smoothing that is applied
#define ATTACK_SMOOTHING 1. // .85
#define DECAY_SMOOTHING .1  //.16//.4

class ControlPotmeter;

class EngineVuMeter : public EngineObject {
    Q_OBJECT
  public:
    EngineVuMeter(const char* );
    virtual ~EngineVuMeter();

    void process(const CSAMPLE* pIn, CSAMPLE* pOut, const int iBufferSize);

  private:
    ControlPotmeter* m_ctrlVuMeter;
    ControlPotmeter* m_ctrlVuMeterL;
    ControlPotmeter* m_ctrlVuMeterR;
    FLOAT_TYPE m_fRMSvolumeL;
    FLOAT_TYPE m_fRMSvolumeSumL;
    FLOAT_TYPE m_fRMSvolumeR;
    FLOAT_TYPE m_fRMSvolumeSumR;
    int m_iSamplesCalculated;

    void doSmooth(FLOAT_TYPE &currentVolume, FLOAT_TYPE newVolume);
};

#endif
