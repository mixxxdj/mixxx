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

//Spectrogram dimensions should be flipped?

#include <dsp/tempogram/NoveltyCurveProcessor.h>
using namespace std;

NoveltyCurveProcessor::NoveltyCurveProcessor(const float &samplingFrequency, const size_t &fftLength,  const size_t &compressionConstant) :
    m_samplingFrequency(samplingFrequency),
    m_fftLength(fftLength),
    m_blockSize(fftLength/2 + 1),
    m_compressionConstant(compressionConstant),
    m_numberOfBands(5),
    m_pBandBoundaries(0),
    m_pBandSum(0)
{
    initialise();
}

NoveltyCurveProcessor::~NoveltyCurveProcessor(){
    cleanup();
}

//allocate all space and set variable
void
NoveltyCurveProcessor::initialise(){
    
    // for bandwise processing, the band is split into 5 bands. m_pBandBoundaries contains the upper and lower bin boundaries for each band.
    m_pBandBoundaries = new int[m_numberOfBands+1];
    m_pBandBoundaries[0] = 0;
    for (unsigned int band = 1; band < m_numberOfBands; band++){
        float lowFreq = 500*pow(2.5, (int)band-1);
        m_pBandBoundaries[band] = m_fftLength*lowFreq/m_samplingFrequency;
	if (m_pBandBoundaries[band] > (int)m_blockSize) {
	    m_pBandBoundaries[band] = m_blockSize;
	}
    }
    m_pBandBoundaries[m_numberOfBands] = m_blockSize;
    m_pBandSum = new float [m_numberOfBands];
}

//delete space allocated in initialise()
void
NoveltyCurveProcessor::cleanup(){
    delete []m_pBandBoundaries;
    m_pBandBoundaries = 0;
    delete []m_pBandSum;
    m_pBandSum = 0;
}

//subtract local average of novelty curve
//uses m_hannWindow as filter
void NoveltyCurveProcessor::subtractLocalAverage(vector<float> &noveltyCurve, const size_t &smoothLength) const
{
    int numberOfBlocks = noveltyCurve.size();
    vector<float> localAverage(numberOfBlocks);
    
    float * m_hannWindow = new float[smoothLength];
    WindowFunction::hanning(m_hannWindow, smoothLength, true);
    
    FIRFilter filter(numberOfBlocks, smoothLength);
    filter.process(&noveltyCurve[0], m_hannWindow, &localAverage[0], FIRFilter::middle);
    
    for (int i = 0; i < numberOfBlocks; i++){
        noveltyCurve[i] -= localAverage[i];
        noveltyCurve[i] = noveltyCurve[i] >= 0 ? noveltyCurve[i] : 0;
    }
    
    delete []m_hannWindow;
    m_hannWindow = 0;
}

//smoothed differentiator filter. Flips upper half of hanning window about y-axis to create coefficients.
void NoveltyCurveProcessor::smoothedDifferentiator(SpectrogramTransposed &spectrogramTransposed, const size_t &smoothLength) const
{
    int numberOfBlocks = spectrogramTransposed[0].size();
    
    float * diffHannWindow = new float [smoothLength];
    WindowFunction::hanning(diffHannWindow, smoothLength, true);
    
    if(smoothLength%2) diffHannWindow[(smoothLength+1)/2 - 1] = 0;
    for(int i = (smoothLength+1)/2; i < (int)smoothLength; i++){
        diffHannWindow[i] = -diffHannWindow[i];
    }
    
    FIRFilter smoothFilter(numberOfBlocks, smoothLength);
    
    for (int i = 0; i < (int)m_blockSize; i++){
        smoothFilter.process(&spectrogramTransposed[i][0], diffHannWindow, &spectrogramTransposed[i][0], FIRFilter::middle);
    }

    delete[] diffHannWindow;
}

//half rectification (set negative to zero)
void NoveltyCurveProcessor::halfWaveRectify(Spectrogram &spectrogram) const
{
    int length = spectrogram.size();
    int height = length > 0 ? spectrogram[0].size() : 0;
    
    for (int i = 0; i < length; i++){
        for (int j = 0; j < height; j++){
            if (spectrogram[i][j] < 0.0) spectrogram[i][j] = 0.0;
        }
    }
}

//process method
vector<float>
NoveltyCurveProcessor::spectrogramToNoveltyCurve(const Spectrogram &spectrogram) const //make argument const &
{
    int numberOfBlocks = spectrogram.size();
    std::vector<float> noveltyCurve(numberOfBlocks);
    SpectrogramTransposed spectrogramTransposed(m_blockSize, vector<float>(spectrogram.size()));
    
    //normalise and log spectrogram
    float normaliseScale = SpectrogramProcessor::calculateMax(spectrogram);
    for (int block = 0; block < (int)numberOfBlocks; block++){
        for (int k = 0; k < (int)m_blockSize; k++){
            float magnitude = spectrogram[block][k];
            if(normaliseScale != 0.0) magnitude /= normaliseScale; //normalise
            spectrogramTransposed[k][block] = log(1+m_compressionConstant*magnitude);
        }
    }
    
    //smooted differentiator
    smoothedDifferentiator(spectrogramTransposed, 5); //make smoothLength a parameter!
    //halfwave rectification
    halfWaveRectify(spectrogramTransposed);
    
    //bandwise processing
    for (int block = 0; block < (int)numberOfBlocks; block++){
        for (int band = 0; band < (int)m_numberOfBands; band++){
            int k = m_pBandBoundaries[band];
            int bandEnd = m_pBandBoundaries[band+1];
            m_pBandSum[band] = 0;
            
            while(k < bandEnd){
                m_pBandSum[band] += spectrogramTransposed[k][block];
                k++;
            }
        }
        float total = 0;
        for(int band = 0; band < (int)m_numberOfBands; band++){
            total += m_pBandSum[band];
        }
        noveltyCurve[block] = total/m_numberOfBands;
    }
    
    //subtract local averages
    subtractLocalAverage(noveltyCurve, 65); //maybe smaller?
    
    return noveltyCurve;
}
