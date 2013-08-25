/***************************************************************************
 *      enginefilter.cpp - Wrapper for FidLib Filter Library	           *
 *			----------------------                             *
 *   copyright      : (C) 2007 by John Sully                               *
 *   email          : jsully@scs.ryerson.ca                                *
 *                                                                         *
 **************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/



#include "engine/enginefilterbutterworth8.h"
#include "engine/enginefilter.h"
#include "engine/engineobject.h"
#include "../lib/fidlib-0.9.10/fidlib.h"

/* Local Prototypes */
inline double _processLowpass(double *coef, double *buf, register double val);
inline double _processBandpass(double *coef, double *buf, register double val);
double inline _processHighpass(double *coef, double *buf, register double val);

inline void zap_buffer_denormals(double *buf, int bufSize) {
	for(int i=0; i < bufSize; i++)
		buf[i] = zap_denormal(buf[i]);
}

EngineFilterButterworth8::EngineFilterButterworth8(int sampleRate) {
    m_sampleRate = sampleRate;
}

EngineFilterButterworth8::~EngineFilterButterworth8() {
}

void EngineFilterButterworth8::initBuffers() {
    for (int i=0; i < m_bufSize; i++) {
        m_buf1[i] = 0;
        m_buf2[i] = 0;
    }
}

inline double _processLowpass(double *coef, double *buf, register double val) {
   register double tmp, fir, iir;
   tmp= buf[0]; memmove(buf, buf+1, 7*sizeof(double));
   iir= val * coef[0];
   iir -= coef[1]*tmp; fir= tmp;
   iir -= coef[2]*buf[0]; fir += buf[0]+buf[0];
   fir += iir;
   tmp= buf[1]; buf[1]= iir; val= fir;
   iir= val;
   iir -= coef[3]*tmp; fir= tmp;
   iir -= coef[4]*buf[2]; fir += buf[2]+buf[2];
   fir += iir;
   tmp= buf[3]; buf[3]= iir; val= fir;
   iir= val;
   iir -= coef[5]*tmp; fir= tmp;
   iir -= coef[6]*buf[4]; fir += buf[4]+buf[4];
   fir += iir;
   tmp= buf[5]; buf[5]= iir; val= fir;
   iir= val;
   iir -= coef[7]*tmp; fir= tmp;
   iir -= coef[8]*buf[6]; fir += buf[6]+buf[6];
   fir += iir;
   buf[7]= iir; val= fir;
   return val;
}

inline double _processBandpass(double *coef, double *buf, register double val) {
   register double tmp, fir, iir;
   tmp= buf[0]; memmove(buf, buf+1, 15*sizeof(double));
   iir= val * coef[0];
   iir -= coef[1]*tmp; fir= tmp;
   iir -= coef[2]*buf[0]; fir += -buf[0]-buf[0];
   fir += iir;
   tmp= buf[1]; buf[1]= iir; val= fir;
   iir= val;
   iir -= coef[3]*tmp; fir= tmp;
   iir -= coef[4]*buf[2]; fir += -buf[2]-buf[2];
   fir += iir;
   tmp= buf[3]; buf[3]= iir; val= fir;
   iir= val;
   iir -= coef[5]*tmp; fir= tmp;
   iir -= coef[6]*buf[4]; fir += -buf[4]-buf[4];
   fir += iir;
   tmp= buf[5]; buf[5]= iir; val= fir;
   iir= val;
   iir -= coef[7]*tmp; fir= tmp;
   iir -= coef[8]*buf[6]; fir += -buf[6]-buf[6];
   fir += iir;
   tmp= buf[7]; buf[7]= iir; val= fir;
   iir= val;
   iir -= coef[9]*tmp; fir= tmp;
   iir -= coef[10]*buf[8]; fir += buf[8]+buf[8];
   fir += iir;
   tmp= buf[9]; buf[9]= iir; val= fir;
   iir= val;
   iir -= coef[11]*tmp; fir= tmp;
   iir -= coef[12]*buf[10]; fir += buf[10]+buf[10];
   fir += iir;
   tmp= buf[11]; buf[11]= iir; val= fir;
   iir= val;
   iir -= coef[13]*tmp; fir= tmp;
   iir -= coef[14]*buf[12]; fir += buf[12]+buf[12];
   fir += iir;
   tmp= buf[13]; buf[13]= iir; val= fir;
   iir= val;
   iir -= coef[15]*tmp; fir= tmp;
   iir -= coef[16]*buf[14]; fir += buf[14]+buf[14];
   fir += iir;
   buf[15]= iir; val= fir;
   return val;
}

double inline _processHighpass(double *coef, double *buf, register double val) {
   register double tmp, fir, iir;
   tmp= buf[0]; memmove(buf, buf+1, 7*sizeof(double));
   iir= val * coef[0];
   iir -= coef[1]*tmp; fir= tmp;
   iir -= coef[2]*buf[0]; fir += -buf[0]-buf[0];
   fir += iir;
   tmp= buf[1]; buf[1]= iir; val= fir;
   iir= val;
   iir -= coef[3]*tmp; fir= tmp;
   iir -= coef[4]*buf[2]; fir += -buf[2]-buf[2];
   fir += iir;
   tmp= buf[3]; buf[3]= iir; val= fir;
   iir= val;
   iir -= coef[5]*tmp; fir= tmp;
   iir -= coef[6]*buf[4]; fir += -buf[4]-buf[4];
   fir += iir;
   tmp= buf[5]; buf[5]= iir; val= fir;
   iir= val;
   iir -= coef[7]*tmp; fir= tmp;
   iir -= coef[8]*buf[6]; fir += -buf[6]-buf[6];
   fir += iir;
   buf[7]= iir; val= fir;
   return val;
}


EngineFilterButterworth8Low::EngineFilterButterworth8Low(int sampleRate,
        double freqCorner1)
    : EngineFilterButterworth8(sampleRate) {
    m_bufSize = 8;
    setFrequencyCorners(freqCorner1);
}

// TODO(XXX) We need to do ramping in the next process() call
// if one or both corners are changed
// https://bugs.launchpad.net/mixxx/+bug/1209294
void EngineFilterButterworth8Low::setFrequencyCorners(double freqCorner1) {
    m_coef[0] = 1 * fid_design_coef(m_coef + 1, 8, "LpBu8", m_sampleRate,
                                    freqCorner1, 0, 0);
    initBuffers();
}

void EngineFilterButterworth8Low::process(const CSAMPLE *pIn,
        const CSAMPLE *ppOut, const int iBufferSize) {
    CSAMPLE *pOutput = (CSAMPLE *)ppOut;

    for (int i=0; i < iBufferSize; i += 2) {
        pOutput[i] = _processLowpass(m_coef, m_buf1, pIn[i]);
        pOutput[i+1] = _processLowpass(m_coef, m_buf2, pIn[i+1]);
    }

    zap_buffer_denormals(m_buf1, m_bufSize);
    zap_buffer_denormals(m_buf2, m_bufSize);
}


EngineFilterButterworth8Band::EngineFilterButterworth8Band(
        int sampleRate, double freqCorner1, double freqCorner2)
    : EngineFilterButterworth8(sampleRate) {
    m_bufSize = 16;
    setFrequencyCorners(freqCorner1, freqCorner2);
}

void EngineFilterButterworth8Band::setFrequencyCorners(double freqCorner1,
        double freqCorner2) {
    m_coef[0]= 1 * fid_design_coef(m_coef + 1, 16, "BpBu8", m_sampleRate,
                                   freqCorner1, freqCorner2, 0);
    initBuffers();
}

void EngineFilterButterworth8Band::process(const CSAMPLE *pIn,
        const CSAMPLE *ppOut, const int iBufferSize) {
    CSAMPLE *pOutput = (CSAMPLE *)ppOut;

    for (int i=0; i < iBufferSize; i += 2) {
        pOutput[i] = _processBandpass(m_coef, m_buf1, pIn[i]);
        pOutput[i+1] = _processBandpass(m_coef, m_buf2, pIn[i+1]);
        if(pOutput[i] != pOutput[i])	//Check for NaN
            pOutput[i] = 0;
        if(pOutput[i+1] != pOutput[i+1])	//Check for NaN
            pOutput[i+1] = 0;
    }

    zap_buffer_denormals(m_buf1, m_bufSize);
    zap_buffer_denormals(m_buf2, m_bufSize);
}


EngineFilterButterworth8High::EngineFilterButterworth8High(int sampleRate,
        double freqCorner1)
    : EngineFilterButterworth8(sampleRate) {
    m_bufSize = 8;
    setFrequencyCorners(freqCorner1);
}

void EngineFilterButterworth8High::setFrequencyCorners(double freqCorner1) {
    m_coef[0] = 1 * fid_design_coef(m_coef + 1, 8, "HpBu8", m_sampleRate,
                                    freqCorner1, 0, 0);
    initBuffers();
}

void EngineFilterButterworth8High::process(const CSAMPLE *pIn,
        const CSAMPLE *ppOut, const int iBufferSize) {
    CSAMPLE *pOutput = (CSAMPLE *)ppOut;

    for (int i=0; i < iBufferSize; i += 2) {
        pOutput[i] = _processHighpass(m_coef, m_buf1, pIn[i]);
        pOutput[i+1] = _processHighpass(m_coef, m_buf2, pIn[i+1]);
    }

    zap_buffer_denormals(m_buf1, m_bufSize);
    zap_buffer_denormals(m_buf2, m_bufSize);
}
