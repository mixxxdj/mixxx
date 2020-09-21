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
                if (readPointer+lag >= inputLength) break;
                else if (readPointer >= 0) {
		    sum += input[readPointer]*input[readPointer+lag];
		}
                readPointer++;
            }
            autocorrelationBlock.push_back(sum/(2*m_windowLength + 1 - lag));
        }
        
        autocorrelation.push_back(autocorrelationBlock);
        readBlockPointerIndex += m_hopSize;
    }
    
    return autocorrelation;
}
