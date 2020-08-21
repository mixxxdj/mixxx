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

#include "AutocorrelationProcessor.h"
using namespace std;
#include <iostream>

AutocorrelationProcessor::AutocorrelationProcessor(int windowLength, int hopSize) :
    m_windowLength(windowLength),
    m_hopSize(hopSize)
{

}

AutocorrelationProcessor::~AutocorrelationProcessor()
{

}

AutoCorrelation AutocorrelationProcessor::process(float * input, int inputLength) const
{
    int readBlockPointerIndex = 0;
    AutoCorrelation autocorrelation;
    
    while(readBlockPointerIndex <= inputLength) {
        
        vector<float> autocorrelationBlock;
        
        for (int lag = 0; lag < m_windowLength; lag++){
            float sum = 0;
            int readPointer = readBlockPointerIndex - m_windowLength/2;
            
            for (int n = 0; n < (int)m_windowLength; n++){
                int refPointer = readPointer + lag;
                if (refPointer >= inputLength) {
                    break;
                } else if (readPointer >= 0) {
                    float diff = input[readPointer]*input[refPointer];
                    float ref1 = input[readPointer]*input[readPointer];
                    float ref2 = input[refPointer]*input[refPointer];
                    float ref = ((ref1 + ref2)/2);
                    if (ref > 0) {
                        sum += diff/((ref1 + ref2)/2);
                    } else {
                        sum += 1;
                    }
                } else if (refPointer < 0) {
                    sum += 1;
                }
                readPointer++;
            }
            autocorrelationBlock.push_back(sum / m_windowLength);
        }
        
        autocorrelation.push_back(autocorrelationBlock);
        readBlockPointerIndex += m_hopSize;
    }
    
    return autocorrelation;
}


AutoCorrelation AutocorrelationProcessor::processPhase(float * input, int inputLength, int hop, int beatsize, int measuresize) const
{
    int readBlockPointerIndex = m_hopSize * hop;
    AutoCorrelation autocorrelation;

    for (int lag = measuresize; lag < m_windowLength; lag += measuresize) {
        vector<float> autocorrelationBlock;
        int readPointer = readBlockPointerIndex - m_windowLength / 2;
        for (int b = 0; b < m_windowLength / beatsize; ++b) {
            float sum = 0;
            for (int n = 0; n < beatsize; n++) {
                int refPointer = readPointer + lag;
                if (refPointer >= inputLength) {
                    break;
                } else if (readPointer >= 0) {
                    float diff = input[readPointer]*input[refPointer];
                    float ref1 = input[readPointer]*input[readPointer];
                    float ref2 = input[refPointer]*input[refPointer];
                    float ref = ((ref1 + ref2)/2);
                    if (ref > 0) {
                        sum += diff/((ref1 + ref2)/2);
                    } else {
                        sum += 1;
                    }
                } else if (refPointer < 0) {
                    sum += 1;
                }
                readPointer++;
            }
            // store
            autocorrelationBlock.push_back(1 - (sum/beatsize));
        }
        autocorrelation.push_back(autocorrelationBlock);
    }
   return autocorrelation;
}

