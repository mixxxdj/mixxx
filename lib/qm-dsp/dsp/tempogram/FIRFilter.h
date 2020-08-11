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

#ifndef __Tempogram__FIRFilter__
#define __Tempogram__FIRFilter__

#include <cmath>
#include <dsp/transforms/FFT.h>
#include <assert.h>
#include <iostream>

class FIRFilter{
public:
    enum OutputTypeArgument{
        first = 0,
        middle,
        all
    };
    
    FIRFilter(const size_t &lengthInput, const size_t &numberOfCoefficients);
    ~FIRFilter();
    void process(const float *pInput, const float *pCoefficients, float * pOutput, OutputTypeArgument outputType = first);
private:
    size_t m_lengthInput;
    size_t m_numberOfCoefficients;
    size_t m_lengthFIRFFT;
    
    double *m_pFftInput;
    double *m_pFftCoefficients;
    double *m_pFftReal1;
    double *m_pFftImag1;
    double *m_pFftReal2;
    double *m_pFftImag2;
    double *m_pFftFilteredReal;
    double *m_pFftFilteredImag;
    double *m_pFftOutputReal;
    double *m_pFftOutputImag;
    FFT *m_fft;
    
    void initialise();
    void cleanup();
};

#endif /* defined(__Tempogram__FIRFilter__) */
