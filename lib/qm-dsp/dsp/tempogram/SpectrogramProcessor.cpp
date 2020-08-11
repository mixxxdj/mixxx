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

#include "SpectrogramProcessor.h"
using namespace std;
#include <iostream>

SpectrogramProcessor::SpectrogramProcessor(const size_t &windowLength, const size_t &fftLength, const size_t &hopSize) :
    m_windowLength(windowLength),
    m_fftLength(fftLength),
    m_hopSize(hopSize),
    m_numberOfOutputBins(ceil(fftLength/2) + 1),
    m_pFftInput(0),
    m_pFftOutputReal(0),
    m_pFftOutputImag(0)
{
    initialise();
}

SpectrogramProcessor::~SpectrogramProcessor(){
    cleanup();
}

void SpectrogramProcessor::initialise(){
    m_fft = new FFTReal(m_fftLength);
    m_pFftInput = new double [m_fftLength];
    m_pFftOutputReal = new double [m_fftLength];
    m_pFftOutputImag = new double [m_fftLength];
}

void SpectrogramProcessor::cleanup(){
    delete []m_pFftInput;
    delete []m_pFftOutputReal;
    delete []m_pFftOutputImag;
    delete m_fft;
    
    m_pFftInput = m_pFftOutputReal = m_pFftOutputImag = 0;
}

SpectrogramTransposed SpectrogramProcessor::transpose(const Spectrogram &spectrogram){
    int numberOfBlocks = spectrogram.size();
    int numberOfBins = spectrogram[0].size();
    
    SpectrogramTransposed spectrogramT(numberOfBins, vector<float>(numberOfBlocks));
    
    for (int i = 0; i < numberOfBlocks; i++){
        for (int j = 0; j < numberOfBins; j++){
            spectrogramT[j][i] = spectrogram[i][j];
        }
    }
    
    return spectrogramT;
}

//calculate max of spectrogram
float SpectrogramProcessor::calculateMax(const Spectrogram &spectrogram)
{
    float max = 0;
    
    int length = spectrogram.size();
    int height = length > 0 ? spectrogram[0].size() : 0;
    
    for (int i = 0; i < length; i++){
        for (int j = 0; j < height; j++){
            max = max > fabs(spectrogram[i][j]) ? max : fabs(spectrogram[i][j]);
        }
    }
    
    return max;
}

//process method
Spectrogram SpectrogramProcessor::process(const float * const pInput, const size_t &inputLength, const float * pWindow) const
{
    Spectrogram spectrogram;
    
    int readBlockPointerIndex = 0;
    int writeBlockPointer = 0;
    
    //cout << m_hopSize << endl;
    while(readBlockPointerIndex <= (int)inputLength) {
        
        int readPointer = readBlockPointerIndex - m_windowLength/2;
        for (int n = 0; n < (int)m_windowLength; n++){
            if(readPointer < 0 || readPointer >= (int)inputLength){
                m_pFftInput[n] = 0.0; //pad with zeros
            }
            else{
                m_pFftInput[n] = pInput[readPointer] * pWindow[n];
            }
            readPointer++;
        }
        for (int n = m_windowLength; n < (int)m_fftLength; n++){
            m_pFftInput[n] = 0.0;
        }
        
        //cerr << m_fftLength << endl;
        m_fft->forward(m_pFftInput, m_pFftOutputReal, m_pFftOutputImag);
        
        vector<float> binValues;
        //@todo: sample at logarithmic spacing? Leave for host?
        for(int k = 0; k < (int)m_numberOfOutputBins; k++){
            binValues.push_back(m_pFftOutputReal[k]*m_pFftOutputReal[k] + m_pFftOutputImag[k]*m_pFftOutputImag[k]); //Magnitude or power?
            //std::cout << spectrogram[k][writeBlockPointer] << std::endl;
        }
        spectrogram.push_back(binValues);
        readBlockPointerIndex += m_hopSize;
        writeBlockPointer++;
    }
    return spectrogram;
}
