/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    QM DSP Library

    Centre for Digital Music, Queen Mary, University of London.
    This file 2005-2006 Christian Landone.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "PhaseVocoder.h"
#include "FFT.h"
#include <math.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PhaseVocoder::PhaseVocoder(unsigned int n) :
    m_n(n)
{
    m_fft = new FFTReal(m_n);
    m_realOut = new double[m_n];
    m_imagOut = new double[m_n];
}

PhaseVocoder::~PhaseVocoder()
{
    delete [] m_realOut;
    delete [] m_imagOut;
    delete m_fft;
}

void PhaseVocoder::FFTShift(unsigned int size, double *src)
{
    const int hs = size/2;
    for (int i = 0; i < hs; ++i) {
        double tmp = src[i];
        src[i] = src[i + hs];
        src[i + hs] = tmp;
    }
}

void PhaseVocoder::process(double *src, double *mag, double *theta)
{
    FFTShift( m_n, src);
	
    m_fft->process(0, src, m_realOut, m_imagOut);

    getMagnitude( m_n/2, mag, m_realOut, m_imagOut);
    getPhase( m_n/2, theta, m_realOut, m_imagOut);
}

void PhaseVocoder::getMagnitude(unsigned int size, double *mag, double *real, double *imag)
{	
    unsigned int j;

    for( j = 0; j < size; j++)
    {
	mag[ j ] = sqrt( real[ j ] * real[ j ] + imag[ j ] * imag[ j ]);
    }
}

void PhaseVocoder::getPhase(unsigned int size, double *theta, double *real, double *imag)
{
    unsigned int k;

    // Phase Angle "matlab" style 
    //Watch out for quadrant mapping  !!!
    for( k = 0; k < size; k++)
    {
	theta[ k ] = atan2( -imag[ k ], real[ k ]);
    }	
}
