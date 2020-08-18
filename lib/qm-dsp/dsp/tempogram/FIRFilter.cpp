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

#include "FIRFilter.h"

using namespace std;

FIRFilter::FIRFilter(const size_t &lengthInput, const size_t &numberOfCoefficients) :
    m_lengthInput(lengthInput),
    m_numberOfCoefficients(numberOfCoefficients),
    m_pFftInput(0),
    m_pFftCoefficients(0),
    m_pFftReal1(0),
    m_pFftImag1(0),
    m_pFftReal2(0),
    m_pFftImag2(0),
    m_pFftFilteredReal(0),
    m_pFftFilteredImag(0),
    m_pFftOutputReal(0),
    m_pFftOutputImag(0)
{
    initialise();
}

FIRFilter::~FIRFilter()
{
    cleanup();
}

//allocate memory
void
FIRFilter::initialise()
{
    //next power of 2
    m_lengthFIRFFT = pow(2,(ceil(log2(m_lengthInput+m_numberOfCoefficients-1))));
    
    m_pFftInput = new double[m_lengthFIRFFT];
    m_pFftCoefficients = new double[m_lengthFIRFFT];
    m_pFftReal1 = new double[m_lengthFIRFFT];
    m_pFftImag1 = new double[m_lengthFIRFFT];
    m_pFftReal2 = new double[m_lengthFIRFFT];
    m_pFftImag2 = new double[m_lengthFIRFFT];
    m_pFftFilteredReal = new double[m_lengthFIRFFT];
    m_pFftFilteredImag = new double[m_lengthFIRFFT];
    m_pFftOutputReal = new double[m_lengthFIRFFT];
    m_pFftOutputImag = new double[m_lengthFIRFFT];
    m_fft = new FFT(m_lengthFIRFFT);
    for(int i = 0; i < (int)m_lengthFIRFFT; i++){
        m_pFftInput[i] = m_pFftCoefficients[i] = m_pFftReal1[i] = m_pFftImag1[i] = m_pFftReal2[i] = m_pFftImag2[i] = m_pFftFilteredReal[i] = m_pFftFilteredImag[i] = m_pFftOutputReal[i] = m_pFftOutputImag[i] = 0.0;
    }
}

void
FIRFilter::process(const float* pInput, const float* pCoefficients, float* pOutput, OutputTypeArgument outputType)
{
    
    //Copy to same length FFT buffers
    for(int i = 0; i < (int)m_lengthFIRFFT; i++){
        m_pFftInput[i] = i < (int)m_lengthInput ? pInput[i] : 0.0;
        m_pFftCoefficients[i] = i < (int)m_numberOfCoefficients ? pCoefficients[i] : 0.0;
    }

    m_fft->process(false, m_pFftInput, 0, m_pFftReal1, m_pFftImag1);
    m_fft->process(false, m_pFftCoefficients, 0 ,m_pFftReal2, m_pFftImag2);
    
    //Multiply FFT coefficients. Multiplication in freq domain is convolution in time domain.
    for (int i = 0; i < (int)m_lengthFIRFFT; i++){
        m_pFftFilteredReal[i] = (m_pFftReal1[i] * m_pFftReal2[i]) - (m_pFftImag1[i] * m_pFftImag2[i]);
        m_pFftFilteredImag[i] = (m_pFftReal1[i] * m_pFftImag2[i]) + (m_pFftReal2[i] * m_pFftImag1[i]);
    }
    
    m_fft->process(true, m_pFftFilteredReal, m_pFftFilteredImag, m_pFftOutputReal, m_pFftOutputImag);
    
    //copy to output
    int offset = 0;
    int outputLength = m_lengthInput;
    if (outputType == all) outputLength = m_lengthInput+m_numberOfCoefficients-1;
    else if (outputType == middle) offset = floor(m_numberOfCoefficients/2.0f);
    else if (outputType != first) cerr << "FIRFilter::process(params) - " << outputType << " is not a valid argument. outputType is set to first." << endl;
    
    for (int i = 0; i < outputLength; i++){
        pOutput[i] = m_pFftOutputReal[i + offset];
    }
}

//remove memory allocations
void
FIRFilter::cleanup()
{
    delete []m_pFftInput;
    delete []m_pFftCoefficients;
    delete []m_pFftReal1;
    delete []m_pFftImag1;
    delete []m_pFftReal2;
    delete []m_pFftImag2;
    delete []m_pFftFilteredReal;
    delete []m_pFftFilteredImag;
    delete []m_pFftOutputReal;
    delete []m_pFftOutputImag;
    delete m_fft;

    m_pFftInput = m_pFftCoefficients = m_pFftReal1 = m_pFftImag1 = m_pFftReal2 = m_pFftImag2 = m_pFftFilteredReal = m_pFftFilteredImag = m_pFftOutputReal = m_pFftOutputImag = 0;
}
