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

#ifndef __Tempogram__Spectrogram__
#define __Tempogram__Spectrogram__
#include <vector>
#include <dsp/transforms/FFT.h>
#include <cmath>
#include <cstddef>

typedef std::vector <std::vector<float> > Spectrogram;
typedef std::vector <std::vector<float> > SpectrogramTransposed;

class SpectrogramProcessor{
    size_t m_windowLength;
    size_t m_fftLength;
    size_t m_hopSize;
    size_t m_numberOfOutputBins;
    double * m_pFftInput;
    double * m_pFftOutputReal;
    double * m_pFftOutputImag;
    FFTReal *m_fft;
    
    void initialise();
    void cleanup();
public:
    SpectrogramProcessor(const size_t &windowLength, const size_t &fftLength, const size_t &hopSize);
    ~SpectrogramProcessor();
    
    Spectrogram process(const float * const pInput, const size_t &inputLength, const float * pWindow) const;
    static SpectrogramTransposed transpose(const Spectrogram &spectrogram);
    static float calculateMax(const Spectrogram &spectrogram);
};

#endif /* defined(__Tempogram__Spectrogram__) */
