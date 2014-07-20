/***************************************************************************
                          enginefilteriir.h  -  description
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

#ifndef ENGINEFILTERIIR_H
#define ENGINEFILTERIIR_H

#define MAX_COEFS 17
#define MAX_INTERNAL_BUF 16

#include "engine/engineobject.h"
#include "util/types.h"

class EngineFilterIIR : public EngineObjectConstIn {
    Q_OBJECT
  public:
    EngineFilterIIR(int bufSize);
    virtual ~EngineFilterIIR();

    void initBuffers();
    virtual void process(const CSAMPLE* pIn, CSAMPLE* pOut,
                         const int iBufferSize) = 0;

  protected:
    int m_sampleRate;

    double m_coef[MAX_COEFS];
    // Old coefficients needed for ramping
    double m_oldCoef[MAX_COEFS];

    int m_bufSize;
    // Channel 1 state
    double m_buf1[MAX_INTERNAL_BUF];
    // Old channel 1 buffer needed for ramping
    double m_oldBuf1[MAX_INTERNAL_BUF];

    // Channel 2 state
    double m_buf2[MAX_INTERNAL_BUF];
    // Old channel 2 buffer needed for ramping
    double m_oldBuf2[MAX_INTERNAL_BUF];

    // Flag set to true if ramping needs to be done
    bool m_doRamping;
};

class EngineFilterIIRLow : public EngineFilterIIR {
    Q_OBJECT
  public:
    EngineFilterIIRLow(int sampleRate, double freqCorner1);
    void setFrequencyCorners(int sampleRate, double freqCorner1);
    void process(const CSAMPLE* pIn, CSAMPLE* pOut, const int iBufferSize);
};

class EngineFilterIIRBand : public EngineFilterIIR {
    Q_OBJECT
  public:
    EngineFilterIIRBand(int sampleRate, double freqCorner1,
                        double freqCorner2);
    void setFrequencyCorners(int sampleRate, double freqCorner1,
                             double freqCorner2);
    void process(const CSAMPLE* pIn, CSAMPLE* pOut, const int iBufferSize);
};

class EngineFilterIIRHigh : public EngineFilterIIR {
    Q_OBJECT
  public:
    EngineFilterIIRHigh(int sampleRate, double freqCorner1);
    void setFrequencyCorners(int sampleRate, double freqCorner1);
    void process(const CSAMPLE* pIn, CSAMPLE* pOut, const int iBufferSize);
};

#endif // ENGINEFILTERIIR_H
