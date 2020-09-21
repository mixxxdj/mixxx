/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
   Vamp Tempogram Plugin
   Carl Bussey, Centre for Digital Music, Queen Mary University of London
   Copyright 2014 Queen Mary University of London.
    
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.  See the file
   COPYING included with this distribution for more information.
*/

#ifndef __Tempogram__NoveltyCurve__
#define __Tempogram__NoveltyCurve__

#include <iostream>
#include <cmath>
#include <vector>
#include <iostream>
#include <dsp/tempogram/FIRFilter.h>
#include "WindowFunction.h"
#include <cassert>
#include <dsp/tempogram/SpectrogramProcessor.h>

class NoveltyCurveProcessor{
    float m_samplingFrequency;
    size_t m_fftLength;
    size_t m_blockSize;
    int m_compressionConstant;
    size_t m_numberOfBands;
    int * m_pBandBoundaries;
    float * m_pBandSum;
    
    void initialise();
    void cleanup();
    void subtractLocalAverage(std::vector<float> &noveltyCurve, const size_t &smoothLength) const;
    void smoothedDifferentiator(SpectrogramTransposed &spectrogram, const size_t &smoothLength) const;
    void halfWaveRectify(SpectrogramTransposed &spectrogram) const;
    
public:
    
    NoveltyCurveProcessor(const float &samplingFrequency, const size_t &fftLength, const size_t &compressionConstant);
    ~NoveltyCurveProcessor();
    std::vector<float> spectrogramToNoveltyCurve(const Spectrogram &spectrogram) const;
};

#endif /* defined(__Tempogram__NoveltyCurve__) */
