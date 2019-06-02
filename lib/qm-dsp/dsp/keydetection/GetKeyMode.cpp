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
#include "maths/MathUtilities.h"
#include "base/Pitch.h"

#include <iostream>

#include <cstring>
#include <cstdlib>

static const int kBinsPerOctave = 36;

// Chords profile
static double MajProfile[kBinsPerOctave] = {
    0.0384, 0.0629, 0.0258, 0.0121, 0.0146, 0.0106, 0.0364, 0.0610, 0.0267,
    0.0126, 0.0121, 0.0086, 0.0364, 0.0623, 0.0279, 0.0275, 0.0414, 0.0186, 
    0.0173, 0.0248, 0.0145, 0.0364, 0.0631, 0.0262, 0.0129, 0.0150, 0.0098,
    0.0312, 0.0521, 0.0235, 0.0129, 0.0142, 0.0095, 0.0289, 0.0478, 0.0239};

static double MinProfile[kBinsPerOctave] = { 
    0.0375, 0.0682, 0.0299, 0.0119, 0.0138, 0.0093, 0.0296, 0.0543, 0.0257,
    0.0292, 0.0519, 0.0246, 0.0159, 0.0234, 0.0135, 0.0291, 0.0544, 0.0248,
    0.0137, 0.0176, 0.0104, 0.0352, 0.0670, 0.0302, 0.0222, 0.0349, 0.0164,
    0.0174, 0.0297, 0.0166, 0.0222, 0.0401, 0.0202, 0.0175, 0.0270, 0.0146};
//
    

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

GetKeyMode::GetKeyMode( int sampleRate, float tuningFrequency,
                        double hpcpAverage, double medianAverage ) :
    m_hpcpAverage( hpcpAverage ),
    m_medianAverage( medianAverage ),
    m_ChrPointer(0),
    m_DecimatedBuffer(0),
    m_ChromaBuffer(0),
    m_MeanHPCP(0),
    m_MajCorr(0),
    m_MinCorr(0),
    m_MedianFilterBuffer(0),
    m_SortedBuffer(0),
    m_keyStrengths(0)
{
    m_DecimationFactor = 8;
        
    // Chromagram configuration parameters
    m_ChromaConfig.normalise = MathUtilities::NormaliseUnitMax;
    m_ChromaConfig.FS = sampleRate/(double)m_DecimationFactor;
    if (m_ChromaConfig.FS < 1) {
        m_ChromaConfig.FS = 1;
    }

    // Set C3 (= MIDI #48) as our base:
    // This implies that key = 1 => Cmaj, key = 12 => Bmaj, key = 13 => Cmin, etc.
    m_ChromaConfig.min = Pitch::getFrequencyForPitch( 48, 0, tuningFrequency );
    m_ChromaConfig.max = Pitch::getFrequencyForPitch( 96, 0, tuningFrequency );

    m_ChromaConfig.BPO = kBinsPerOctave;
    m_ChromaConfig.CQThresh = 0.0054;

    // Chromagram inst.
    m_Chroma = new Chromagram( m_ChromaConfig );

    // Get calculated parameters from chroma object
    m_ChromaFrameSize = m_Chroma->getFrameSize();
    // override hopsize for this application
    m_ChromaHopSize = m_ChromaFrameSize;

//    std::cerr << "chroma frame size = " << m_ChromaFrameSize << ", decimation factor = " << m_DecimationFactor << " therefore block size = " << getBlockSize() << std::endl;

    // Chromagram average and estimated key median filter lengths
    m_ChromaBuffersize = (int)ceil( m_hpcpAverage * m_ChromaConfig.FS/m_ChromaFrameSize );
    m_MedianWinsize = (int)ceil( m_medianAverage * m_ChromaConfig.FS/m_ChromaFrameSize );
    
    // Reset counters
    m_bufferindex = 0;
    m_ChromaBufferFilling = 0;
    m_MedianBufferFilling = 0;

    // Spawn objectc/arrays
    m_DecimatedBuffer = new double[m_ChromaFrameSize];
    
    m_ChromaBuffer = new double[kBinsPerOctave * m_ChromaBuffersize];
    memset( m_ChromaBuffer, 0, sizeof(double) * kBinsPerOctave * m_ChromaBuffersize);
    
    m_MeanHPCP = new double[kBinsPerOctave];
    
    m_MajCorr = new double[kBinsPerOctave];
    m_MinCorr = new double[kBinsPerOctave];
    
    m_MajProfileNorm = new double[kBinsPerOctave];
    m_MinProfileNorm = new double[kBinsPerOctave];

    double mMaj = MathUtilities::mean( MajProfile, kBinsPerOctave );
    double mMin = MathUtilities::mean( MinProfile, kBinsPerOctave );

    for( unsigned int i = 0; i < kBinsPerOctave; i++ ) {
        m_MajProfileNorm[i] = MajProfile[i] - mMaj;
        m_MinProfileNorm[i] = MinProfile[i] - mMin;
    }

    m_MedianFilterBuffer = new int[ m_MedianWinsize ];
    memset( m_MedianFilterBuffer, 0, sizeof(int)*m_MedianWinsize);
    
    m_SortedBuffer = new int[ m_MedianWinsize ];
    memset( m_SortedBuffer, 0, sizeof(int)*m_MedianWinsize);
    
    m_Decimator = new Decimator( m_ChromaFrameSize*m_DecimationFactor, m_DecimationFactor );

    m_keyStrengths = new double[24];
}

GetKeyMode::~GetKeyMode()
{
    delete m_Chroma;
    delete m_Decimator;
    
    delete [] m_DecimatedBuffer;
    delete [] m_ChromaBuffer;
    delete [] m_MeanHPCP;
    delete [] m_MajCorr;
    delete [] m_MinCorr;
    delete [] m_MajProfileNorm;
    delete [] m_MinProfileNorm;
    delete [] m_MedianFilterBuffer;
    delete [] m_SortedBuffer;
    delete [] m_keyStrengths;
}

double GetKeyMode::krumCorr( const double *pDataNorm, const double *pProfileNorm, 
                             int shiftProfile, unsigned int length)
{
    double retVal= 0.0;
    
    double num = 0;
    double den = 0;
    double sum1 = 0;
    double sum2 = 0;
    
    for( unsigned int i = 0; i <length; i++ )
    {
        int k = (i - shiftProfile + length) % length;

        num += pDataNorm[i] * pProfileNorm[k];

        sum1 += ( pDataNorm[i] * pDataNorm[i] );
        sum2 += ( pProfileNorm[k] * pProfileNorm[k] );
    }
	
    den = sqrt(sum1 * sum2);

    if( den>0 ) {
        retVal = num/den;
    } else {
        retVal = 0;
    }

    return retVal;
}

int GetKeyMode::process(double *PCMData)
{
    int key;
    unsigned int j,k;

    //////////////////////////////////////////////
    m_Decimator->process( PCMData, m_DecimatedBuffer);

    m_ChrPointer = m_Chroma->process( m_DecimatedBuffer );

/*
    std::cout << "raw chroma: ";
    for (int ii = 0; ii < kBinsPerOctave; ++ii) {
      if (ii % (kBinsPerOctave/12) == 0) std::cout << "\n";
        std::cout << m_ChrPointer[ii] << " ";
    }
    std::cout << std::endl;
*/
    // populate hpcp values;
    int cbidx;
    for( j = 0; j < kBinsPerOctave; j++ ) {
        cbidx = (m_bufferindex * kBinsPerOctave) + j;
        m_ChromaBuffer[ cbidx ] = m_ChrPointer[j];
    }

    //keep track of input buffers;
    if( m_bufferindex++ >= m_ChromaBuffersize - 1) {
        m_bufferindex = 0;
    }

    // track filling of chroma matrix
    if( m_ChromaBufferFilling++ >= m_ChromaBuffersize) {
        m_ChromaBufferFilling = m_ChromaBuffersize;
    }

    //calculate mean
    for( k = 0; k < kBinsPerOctave; k++ ) {
        double mnVal = 0.0;
        for( j = 0; j < m_ChromaBufferFilling; j++ ) {
            mnVal += m_ChromaBuffer[ k + (j*kBinsPerOctave) ];
        }

        m_MeanHPCP[k] = mnVal/(double)m_ChromaBufferFilling;
    }

    // Normalize for zero average
    double mHPCP = MathUtilities::mean( m_MeanHPCP, kBinsPerOctave );
    for( k = 0; k < kBinsPerOctave; k++ )
    {
        m_MeanHPCP[k] -= mHPCP;
    }


    for( k = 0; k < kBinsPerOctave; k++ )
    {
        // The Cromagram has the center of C at bin 0, while the major
        // and minor profiles have the center of C at 1. We want to have
        // the correlation for C result also at 1.
        // To achieve this we have to shift two times:
        m_MajCorr[k] = krumCorr( m_MeanHPCP, m_MajProfileNorm, (int)k - 2, kBinsPerOctave );
        m_MinCorr[k] = krumCorr( m_MeanHPCP, m_MinProfileNorm, (int)k - 2, kBinsPerOctave );
    }

/*
  std::cout << "raw keys: ";
  for (int ii = 0; ii < kBinsPerOctave; ++ii) {
      if (ii % (kBinsPerOctave/12) == 0) std::cout << "\n";
      std::cout << m_MajCorr[ii] << " ";
  }
  for (int ii = 0; ii < kBinsPerOctave; ++ii) {
      if (ii % (kBinsPerOctave/12) == 0) std::cout << "\n";
      std::cout << m_MinCorr[ii] << " ";
  }
  std::cout << std::endl;
*/

    // m_MajCorr[1] is C center  1 / 3 + 1 = 1
    // m_MajCorr[4] is D center  4 / 3 + 1 = 2
    // '+ 1' because we number keys 1-24, not 0-23.
    double maxMaj;
    int maxMajBin = MathUtilities::getMax( m_MajCorr, kBinsPerOctave, &maxMaj );
    double maxMin;
    int maxMinBin = MathUtilities::getMax( m_MinCorr, kBinsPerOctave, &maxMin );
    int maxBin = (maxMaj > maxMin) ? maxMajBin : (maxMinBin + kBinsPerOctave);
    key = maxBin / 3 + 1;

//    std::cout << "fractional key pre-sorting: " << (maxBin + 2) / 3.0 << std::endl;
//    std::cout << "key pre-sorting: " << key << std::endl;


    //Median filtering

    // track Median buffer initial filling
    if( m_MedianBufferFilling++ >= m_MedianWinsize) {
        m_MedianBufferFilling = m_MedianWinsize;
    }

    //shift median buffer
    for( k = 1; k < m_MedianWinsize; k++ ) {
        m_MedianFilterBuffer[ k - 1 ] = m_MedianFilterBuffer[ k ];
    }

    //write new key value into median buffer
    m_MedianFilterBuffer[ m_MedianWinsize - 1 ] = key;


    //Copy median into sorting buffer, reversed
    unsigned int ijx = 0;
    for( k = 0; k < m_MedianWinsize; k++ ) {
        m_SortedBuffer[k] = m_MedianFilterBuffer[m_MedianWinsize-1-ijx];
        ijx++;
    }

    qsort(m_SortedBuffer, m_MedianBufferFilling, sizeof(unsigned int),
          MathUtilities::compareInt);
/*
  std::cout << "sorted: ";
  for (int ii = 0; ii < m_MedianBufferFilling; ++ii) {
  std::cout << m_SortedBuffer[ii] << " ";
  }
  std::cout << std::endl;
*/
    int sortlength = m_MedianBufferFilling;
    int midpoint = (int)ceil((double)sortlength/2);

//  std::cout << "midpoint = " << midpoint << endl;

    if( midpoint <= 0 ) {
        midpoint = 1;
    }

    key = m_SortedBuffer[midpoint-1];

// std::cout << "returning key = " << key << endl;

    return key;
}


bool GetKeyMode::isModeMinor( int key )
{ 
    return (key > 12);
}

unsigned int getChromaSize() 
{ 
    return kBinsPerOctave; 
}

double* GetKeyMode::getKeyStrengths() {
    unsigned int k;

    for (k = 0; k < 24; ++k) {
        m_keyStrengths[k] = 0;
    }

    for( k = 0; k < kBinsPerOctave; k++ )
    {
        int idx = k / (kBinsPerOctave/12);
        int rem = k % (kBinsPerOctave/12);
        if (rem == 0 || m_MajCorr[k] > m_keyStrengths[idx]) {
            m_keyStrengths[idx] = m_MajCorr[k];
        }
    }

    for( k = 0; k < kBinsPerOctave; k++ )
    {
        int idx = (k + kBinsPerOctave) / (kBinsPerOctave/12);
        int rem = k % (kBinsPerOctave/12);
        if (rem == 0 || m_MinCorr[k] > m_keyStrengths[idx]) {
            m_keyStrengths[idx] = m_MinCorr[k];
        }
    }

/*
    std::cout << "key strengths: ";
    for (int ii = 0; ii < 24; ++ii) {
        if (ii % 6 == 0) std::cout << "\n";
        std::cout << m_keyStrengths[ii] << " ";
    }
    std::cout << std::endl;
*/

    return m_keyStrengths;
}
