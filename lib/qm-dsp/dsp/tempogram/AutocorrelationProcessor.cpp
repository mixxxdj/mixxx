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
        
        std::vector<float> autocorrelationBlock;
        
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

int AutocorrelationProcessor::processOffset(float * input, int inputLength, int hop, const std::vector<int>& periods) const
{
    int readBlockPointerIndex = m_hopSize * hop - m_windowLength / 2;
    AutoCorrelation autocorrelation;

    float max_sum = 0;
    int max_lag = 0;
    // periods[0] is most likely 1/8 expect a maximum at least at 1/4
    int shift = periods[0];
    if (shift < 20) {
        // Look at least into a 680 ms Window (88 BPM)
        shift = 20;
    }
    for (int lag = -shift; lag < shift * 2; ++lag) {
        int readPointer = readBlockPointerIndex + lag;
        float sum = 0;
        if (readPointer >= 0) {
            sum = input[readPointer];
        }
        for (int i = 0; i < periods.size(); ++i) {
            int period = periods[i];
            if (readPointer + period + 1 >= inputLength) {
                break;
            }
            if (readPointer + period - 1 >= 0) {
                float itSum = input[readPointer + period - 1];
                itSum += input[readPointer + period];
                itSum += input[readPointer + period + 1];
                sum += itSum / 3;
            }
        }
        if (sum > max_sum) {
            max_sum = sum;
            max_lag = lag;
        }
    }
    return max_lag;
}


AutoCorrelation AutocorrelationProcessor::processChanges(float * input, int inputLength, int hop, int beatsize, int measuresize, int offset) const
{
    int readBlockPointerIndex = m_hopSize * hop + offset - 1;
    AutoCorrelation autocorrelation;

    for (int lag = measuresize; lag < m_windowLength; lag += measuresize) {
        std::vector<float> autocorrelationBlock;
        int readPointer = readBlockPointerIndex - m_windowLength / 2;
        for (int b = 0; b < m_windowLength / beatsize; ++b) {
            float sum = 0;
            for (int n = 0; n <= beatsize; n++) {
                int refPointer = readPointer + lag;
                if (refPointer + 1 >= inputLength) {
                    break;
                } else if (readPointer >= 0) {
                    float inref = (input[refPointer - 1] + input[refPointer] + input[refPointer + 1]) / 3;
                    float diff = input[readPointer] * inref;
                    float ref1 = input[readPointer] * input[readPointer];
                    float ref2 = inref * inref;
                    float ref = ((ref1 + ref2)/2);
                    if (ref > 0) {
                        sum += diff/((ref1 + ref2)/2);
                    } else {
                        sum += 1;
                    }
                } else if (refPointer > 0) {
                    sum += 1;
                }
                readPointer++;
            }

            // store
            autocorrelationBlock.push_back(1 - (sum / (beatsize + 1)));


            /*
            float sumread = 0;
            float sumref = 0;
            for (int n = 0; n <= beatsize + 1; n++) {
                int refPointer = readPointer + lag;
                if (refPointer >= inputLength) {
                    break;
                } else if (readPointer >= 0) {
                    sumread += input[readPointer];
                    sumref += input[refPointer];
                } else if (refPointer > 0) {
                    sumref += 1;
                }
                readPointer++;
            }
            float diff = sumread * sumref;
            float ref1 = sumread * sumread;
            float ref2 = sumref * sumref;
            float ref = ((ref1 + ref2)/2);
            if (ref > 0) {
                sum += diff/((ref1 + ref2)/2);
            } else {
                sum += 1;
            }
            // store
            autocorrelationBlock.push_back(1 - sum);
            */
        }
        autocorrelation.push_back(autocorrelationBlock);
    }
   return autocorrelation;
}


std::vector<std::pair<int, float> > AutocorrelationProcessor::processPhase(float * input, int inputLength, int hop, int beatsize, int measuresize, int offset) const
{
    int readBlockPointerIndex = m_hopSize * hop - m_windowLength / 2 + offset - 3;

    int shift = measuresize / beatsize / 2 + 1;
    std::vector<std::pair<int, float> > autocorrelationBlock;

    for (int lag = -shift * beatsize; lag <= m_windowLength - measuresize; lag += beatsize) {
        int readPointer = readBlockPointerIndex + lag;
        int map_index = readPointer;
        float sum = 0;
        int refPointer = readPointer + measuresize;
        for (int n = 0; n < measuresize + 1; n++) {
            if (refPointer >= inputLength) {
                break;
            } else if (readPointer >= 0) {
                float inref = input[refPointer];
                float diff = input[readPointer] * inref;
                float ref1 = input[readPointer] * input[readPointer];
                float ref2 = inref * inref;
                float ref = ((ref1 + ref2)/2);
                if (ref > 0) {
                    sum += diff /((ref1 + ref2)/2);
                } else {
                    sum += 1;
                }
            } else if (refPointer < 0) {
                sum += 1;
            }
            readPointer++;
            refPointer++;
        }
        // store
        autocorrelationBlock.push_back(std::pair(map_index, sum / (measuresize + 1)));
    }
    return autocorrelationBlock;
}


std::vector<float> AutocorrelationProcessor::processPhase2(float * input, int inputLength, std::set<int> measuresizes, int offset) const
{
    int readBlockPointerIndex = offset - 3;
    std::vector<float> autocorrelation;

    for (int measuresize: measuresizes) {
        int readPointer = readBlockPointerIndex;
        float sum = 0;
        int refPointer = readPointer + measuresize;
        for (int n = 0; n < measuresize - 3; n++) {
            if (refPointer >= inputLength) {
                break;
            } else if (readPointer >= 0) {
                float inref = input[refPointer];
                float diff = input[readPointer] * inref;
                float ref1 = input[readPointer] * input[readPointer];
                float ref2 = inref * inref;
                float ref = ((ref1 + ref2)/2);
                if (ref > 0) {
                    sum += diff /((ref1 + ref2)/2);
                } else {
                    sum += 1;
                }
            } else if (refPointer < 0) {
                sum += 1;
            }
            readPointer++;
            refPointer++;
        }
        // store
        autocorrelation.push_back(sum / (measuresize - 3));
    }
    return autocorrelation;
}

int AutocorrelationProcessor::findBeat(float * input, int inputLength, const std::vector<int>& periods, int initalOffset) const
{
    int readBlockPointerIndex = initalOffset;
    AutoCorrelation autocorrelation;

    float max_sum = 0;
    int max_lag = 0;
    // periods[0] is most likely 1/8 expect a maximum at least at 1/4
    int shift = periods[0];
    if (shift < 20) {
        // Look at least into a 227 ms Window
        shift = 20;
    }
    std::cout << "AutocorrelationProcessor::findBeat " << initalOffset << std::endl;
    for (int lag = 0; lag < 500; ++lag) {
        int readPointer = readBlockPointerIndex + lag;
        float sum = 0;
        if (readPointer >= 0) {
            sum = input[readPointer] * 4;
        }
        for (int i = 0; i < periods.size(); ++i) {
            int period = periods[i];
            int periodReadPinter = readPointer + period;
            if (periodReadPinter < inputLength && periodReadPinter >= 0) {
                sum += input[readPointer + period];
            }
            periodReadPinter = readPointer - period;
            if (periodReadPinter < inputLength && periodReadPinter >= 0) {
                sum += input[readPointer + period];
            }
        }
        if (sum > max_sum) {
            max_sum = sum;
            max_lag = lag;
        }
        if (readPointer >= 0) {
            std::cout << input[readPointer] << " " << sum << std::endl;
        }
    }
    return max_lag;
}

