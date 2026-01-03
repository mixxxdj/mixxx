/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */
/*
    QM DSP Library

    Centre for Digital Music, Queen Mary, University of London.
    This file 2005-2006 Christian Landone and Katy Noland.

    Fixes to correct chroma offsets and for thread safety contributed
    by Daniel Schürmann.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "GetKeyMode.h"

#include "dsp/rateconversion/Decimator.h"
#include "dsp/chromagram/Chromagram.h"

#include "maths/MathUtilities.h"
#include "base/Pitch.h"

#include <algorithm>
#include <iostream>

#include <cstring>
#include <cstdlib>

namespace {

constexpr int kBinsPerOctave = 36;
constexpr double kBandwithFactor = pow(2, 1.0 / kBinsPerOctave);

// Chords profile
double MajProfile[kBinsPerOctave] = {
    0.0384, 0.0629, 0.0258, 0.0121, 0.0146, 0.0106, 0.0364, 0.0610, 0.0267,
    0.0126, 0.0121, 0.0086, 0.0364, 0.0623, 0.0279, 0.0275, 0.0414, 0.0186, 
    0.0173, 0.0248, 0.0145, 0.0364, 0.0631, 0.0262, 0.0129, 0.0150, 0.0098,
    0.0312, 0.0521, 0.0235, 0.0129, 0.0142, 0.0095, 0.0289, 0.0478, 0.0239
};

double MinProfile[kBinsPerOctave] = {
    0.0375, 0.0682, 0.0299, 0.0119, 0.0138, 0.0093, 0.0296, 0.0543, 0.0257,
    0.0292, 0.0519, 0.0246, 0.0159, 0.0234, 0.0135, 0.0291, 0.0544, 0.0248,
    0.0137, 0.0176, 0.0104, 0.0352, 0.0670, 0.0302, 0.0222, 0.0349, 0.0164,
    0.0174, 0.0297, 0.0166, 0.0222, 0.0401, 0.0202, 0.0175, 0.0270, 0.0146
};

double safeLog(double x) {
    return std::log(std::max(x, 1e-12));
};

double getGaussianPeak(double* pChromaBuffer, int peakBin) {
    double y0s = 0.0;
    double y1s = 0.0;
    double y2s = 0.0;
    for (int i = 0; i < kBinsPerOctave / 3; ++i) {
        y0s += pChromaBuffer[i * 3];
        y1s += pChromaBuffer[i * 3 + 1];
        y2s += pChromaBuffer[i * 3 + 2];
    }

    // std::cout << "getGaussianPeak 1: " << y0s << " " << y1s << " " << y2s << " " << peakBin << std::endl;

    int modPeak = peakBin  % 3; // 1 for 440 Hz
    double y0;
    double y1;
    double y2;
    if (y0s > y1s && y0s > y2s) {
        y0 = y2s;
        y1 = y0s;
        y2 = y1s;
        if (modPeak == 1) {
            peakBin -= 1;
        } else if (modPeak == 2) {
            peakBin += 1;
        }
    } else if (y1s > y0s && y1s > y2s) {
        y0 = y0s;
        y1 = y1s;
        y2 = y2s;
        if (modPeak == 0) {
            peakBin += 1;
        } else if (modPeak == 2) {
            peakBin -= 1;
        }
    } else {
        y0 = y1s;
        y1 = y2s;
        y2 = y0s;
        if (modPeak == 0) {
            peakBin -= 1;
        } else if (modPeak == 1) {
            peakBin -= 1;
        }
    }

    // std::cout << "getGaussianPeak 1: " << y0 << " " << y1 << " " << y2 << std::endl;

    y0 = safeLog(y0 * kBandwithFactor);
    y1 = safeLog(y1);
    y2 = safeLog(y2 / kBandwithFactor);

    // std::cout << "getGaussianPeak 2: " << y0 << " " << y1 << " " << y2 << std::endl;

    double denom = (y0 - 2.0 * y1 + y2);
    if (denom == 0) {
        return peakBin;
    }
    return peakBin + (y0 - y2) / (2 * denom);
}

} // namespace
    

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

GetKeyMode::GetKeyMode(Config config) :
    m_hpcpAverage(config.hpcpAverage),
    m_medianAverage(config.medianAverage),
    m_decimationFactor(config.decimationFactor),
    m_chrPointer(0),
    m_decimatedBuffer(0),
    m_chromaBuffer(0),
    m_meanHPCP(0),
    m_majCorr(0),
    m_minCorr(0),
    m_medianFilterBuffer(0),
    m_sortedBuffer(0),
    m_keyStrengths(0)
{
    ChromaConfig chromaConfig;
    
    // Chromagram configuration parameters
    chromaConfig.normalise = MathUtilities::NormaliseUnitMax;
    chromaConfig.FS = config.sampleRate / (double)m_decimationFactor;
    if (chromaConfig.FS < 1) {
        chromaConfig.FS = 1;
    }

    // Set C3 (= MIDI #48) 130,8 Hz as our base:
    // This implies that key = 1 => Cmaj, key = 12 => Bmaj, key = 13 => Cmin, etc.
    const float centsOffset = -12.0f / kBinsPerOctave * 100; // 3 bins per note, start with the first
    chromaConfig.min =
        Pitch::getFrequencyForPitch( 48, centsOffset, config.tuningFrequency );
    // Set C7 (= MIDI #96) 2093 Hz
    chromaConfig.max =
        Pitch::getFrequencyForPitch( 96, centsOffset, config.tuningFrequency );

    chromaConfig.BPO = kBinsPerOctave;
    chromaConfig.CQThresh = 0.0054;

    // Chromagram inst.
    m_chroma = new Chromagram(chromaConfig);

    // Get calculated parameters from chroma object
    m_chromaFrameSize = m_chroma->getFrameSize();

    // override hopsize for this application
    m_chromaHopSize = m_chromaFrameSize / config.frameOverlapFactor;

//    std::cerr << "chroma frame size = " << m_ChromaFrameSize << ", decimation factor = " << m_DecimationFactor << " therefore block size = " << getBlockSize() << std::endl;

    // Chromagram average and estimated key median filter lengths
    m_chromaBufferSize = (int)ceil
        (m_hpcpAverage * chromaConfig.FS / m_chromaFrameSize);
    m_medianWinSize = (int)ceil
        (m_medianAverage * chromaConfig.FS / m_chromaFrameSize);
    
    // Reset counters
    m_bufferIndex = 0;
    m_chromaBufferFilling = 0;
    m_medianBufferFilling = 0;

    // Spawn objectc/arrays
    m_decimatedBuffer = new double[m_chromaFrameSize];
    m_chromaBuffer = new double[kBinsPerOctave * m_chromaBufferSize];

    memset(m_chromaBuffer, 0,
           sizeof(double) * kBinsPerOctave * m_chromaBufferSize);
    
    m_meanHPCP = new double[kBinsPerOctave];
    
    m_majCorr = new double[kBinsPerOctave];
    m_minCorr = new double[kBinsPerOctave];
    
    m_majProfileNorm = new double[kBinsPerOctave];
    m_minProfileNorm = new double[kBinsPerOctave];

    double mMaj = MathUtilities::mean( MajProfile, kBinsPerOctave );
    double mMin = MathUtilities::mean( MinProfile, kBinsPerOctave );

    for (int i = 0; i < kBinsPerOctave; i++) {
        m_majProfileNorm[i] = MajProfile[i] - mMaj;
        m_minProfileNorm[i] = MinProfile[i] - mMin;
    }

    m_medianFilterBuffer = new int[ m_medianWinSize ];
    memset( m_medianFilterBuffer, 0, sizeof(int)*m_medianWinSize);
    
    m_sortedBuffer = new int[ m_medianWinSize ];
    memset( m_sortedBuffer, 0, sizeof(int)*m_medianWinSize);
    
    m_decimator = new Decimator( m_chromaFrameSize * m_decimationFactor,
                                 m_decimationFactor );

    m_keyStrengths = new double[24];
}

GetKeyMode::~GetKeyMode()
{
    delete m_chroma;
    delete m_decimator;
    
    delete [] m_decimatedBuffer;
    delete [] m_chromaBuffer;
    delete [] m_meanHPCP;
    delete [] m_majCorr;
    delete [] m_minCorr;
    delete [] m_majProfileNorm;
    delete [] m_minProfileNorm;
    delete [] m_medianFilterBuffer;
    delete [] m_sortedBuffer;
    delete [] m_keyStrengths;
}

double GetKeyMode::krumCorr( const double *pDataNorm, const double *pProfileNorm, 
                             int shiftProfile, int length)
{
    double retVal= 0.0;
    
    double num = 0;
    double den = 0;
    double sum1 = 0;
    double sum2 = 0;
    
    for (int i = 0; i < length; i++) {

        int k = (i - shiftProfile + length) % length;

        num += pDataNorm[i] * pProfileNorm[k];

        sum1 += (pDataNorm[i] * pDataNorm[i]);
        sum2 += (pProfileNorm[k] * pProfileNorm[k]);
    }
        
    den = sqrt(sum1 * sum2);

    if (den > 0) {
        retVal = num/den;
    } else {
        retVal = 0;
    }

    return retVal;
}

int GetKeyMode::process(double *pcmData)
{
    int key;
    int j, k;

    m_decimator->process(pcmData, m_decimatedBuffer);

    m_chrPointer = m_chroma->process(m_decimatedBuffer);

    // populate hpcp values
    int cbidx;
    for (j = 0;j < kBinsPerOctave;j++ ) {
        cbidx = (m_bufferIndex * kBinsPerOctave) + j;
        m_chromaBuffer[ cbidx ] = m_chrPointer[j];
    }

    // keep track of input buffers
    if (m_bufferIndex++ >= m_chromaBufferSize - 1) {
        m_bufferIndex = 0;
    }

    // track filling of chroma matrix
    if (m_chromaBufferFilling++ >= m_chromaBufferSize) {
        m_chromaBufferFilling = m_chromaBufferSize;
    }

    // calculate mean
    for (k = 0; k < kBinsPerOctave; k++) {
        double mnVal = 0.0;
        for (j = 0; j < m_chromaBufferFilling; j++) {
            mnVal += m_chromaBuffer[ k + (j * kBinsPerOctave) ];
        }

        m_meanHPCP[k] = mnVal / (double)m_chromaBufferFilling;
    }

    // Normalize for zero average
    double mHPCP = MathUtilities::mean(m_meanHPCP, kBinsPerOctave);
    for (k = 0; k < kBinsPerOctave; k++) {
        m_meanHPCP[k] -= mHPCP;
    }

    for (k = 0; k < kBinsPerOctave; k++) {
        // The cromagram and the major and minor profiles have the
        // center of C at bin 1. We want to have the correlation for C result
        // also at 1. To achieve this we have to shift by one:
        m_majCorr[k] = krumCorr(m_meanHPCP, m_majProfileNorm, (int)k - 1, kBinsPerOctave);
        m_minCorr[k] = krumCorr(m_meanHPCP, m_minProfileNorm, (int)k - 1, kBinsPerOctave);
    }

    // m_MajCorr[1] is C center  1 / 3 + 1 = 1
    // m_MajCorr[4] is D center  4 / 3 + 1 = 2
    // '+ 1' because we number keys 1-24, not 0-23.
    double maxMaj;
    int maxMajBin = MathUtilities::getMax(m_majCorr, kBinsPerOctave, &maxMaj);
    double maxMin;
    int maxMinBin = MathUtilities::getMax(m_minCorr, kBinsPerOctave, &maxMin);
    int maxBin = (maxMaj > maxMin) ? maxMajBin : (maxMinBin + kBinsPerOctave);
    key = maxBin / 3 + 1;

    // Median filtering

    // track Median buffer initial filling
    if (m_medianBufferFilling++ >= m_medianWinSize) {
        m_medianBufferFilling = m_medianWinSize;
    }

    // rotate right
    std::rotate(m_medianFilterBuffer, m_medianFilterBuffer + m_medianWinSize - 1, m_medianFilterBuffer + m_medianWinSize);

    // write new key value into median buffer
    m_medianFilterBuffer[0] = key;

    // copy median into sorting buffer, reversed
    std::copy(m_medianFilterBuffer, m_medianFilterBuffer + m_medianBufferFilling, m_sortedBuffer);
    const std::size_t medianIndex = m_medianBufferFilling / 2;

    std::nth_element(
        m_sortedBuffer,
        m_sortedBuffer + medianIndex,
        m_sortedBuffer + m_medianBufferFilling
    );

    key = m_sortedBuffer[medianIndex];
    return key;
}

double* GetKeyMode::getKeyStrengths() {
    int k;

    for (k = 0; k < 24; ++k) {
        m_keyStrengths[k] = 0;
    }

    for (k = 0; k < kBinsPerOctave; k++) {
        int idx = k / (kBinsPerOctave/12);
        int rem = k % (kBinsPerOctave/12);
        if (rem == 0 || m_majCorr[k] > m_keyStrengths[idx]) {
            m_keyStrengths[idx] = m_majCorr[k];
        }
    }

    for (k = 0; k < kBinsPerOctave; k++) {
        int idx = (k + kBinsPerOctave) / (kBinsPerOctave/12);
        int rem = k % (kBinsPerOctave/12);
        if (rem == 0 || m_minCorr[k] > m_keyStrengths[idx]) {
            m_keyStrengths[idx] = m_minCorr[k];
        }
    }

    return m_keyStrengths;
}

