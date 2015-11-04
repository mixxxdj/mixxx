/***************************************************************************
                          enginefilterblock.h  -  description
                             -------------------
    begin                : Thu Apr 4 2002
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

#ifndef ENGINEFILTERBLOCK_H
#define ENGINEFILTERBLOCK_H

#include "engine/engineobject.h"
#include "engine/enginefilterbessel4.h"
#include "engine/enginefilterbutterworth8.h"

class ControlObjectSlave;
class ControlLogpotmeter;
class ControlPotmeter;
class ControlPushButton;

#define SIZE_NOISE_BUF 40
//#define NOISE_FACTOR 116.415321827e-12 // 1/4 bit of noise (99db SNR)
#define NOISE_FACTOR 0.25              // this is necessary to prevent denormals
// from consuming too much CPU resources
// and is well below being audible.
/**
  * Parallel processing of LP, BP and HP filters, and final mixing
  *
  *@author Tue and Ken Haste Andersen
  */

class EngineFilterBlock : public EngineObject {
    Q_OBJECT
  public:
    EngineFilterBlock(const char* group);
    virtual ~EngineFilterBlock();

    void process(CSAMPLE* pInOut, const int iBufferSize);

  private:
    void setFilters();

    CSAMPLE *m_pLowBuf, *m_pBandBuf, *m_pHighBuf;
    EngineFilterBessel4Low* lowLight;
    EngineFilterBessel4Band* bandLight;
    EngineFilterBessel4High* highLight;
    EngineFilterButterworth8Low* lowDef;
    EngineFilterButterworth8Band* bandDef;
    EngineFilterButterworth8High* highDef;
    EngineObjectConstIn *low, *band, *high;
    ControlLogpotmeter *filterpotLow, *filterpotMid, *filterpotHigh;
    ControlPushButton *filterKillLow, *filterKillMid, *filterKillHigh;
    ControlObjectSlave* m_pSampleRate;

    static ControlPotmeter *s_loEqFreq, *s_hiEqFreq;
    static ControlPushButton *s_lofiEq;
    static ControlPushButton *s_EnableEq;

    int m_iOldSampleRate;
    double old_low, old_mid, old_high, old_dry;

    int ilowFreq, ihighFreq;
    bool blofi;
    bool m_eqNeverTouched;
};

#endif
