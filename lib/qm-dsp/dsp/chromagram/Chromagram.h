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

#ifndef CHROMAGRAM_H
#define CHROMAGRAM_H

#include "dsp/transforms/FFT.h"
#include "base/Window.h"
#include "ConstantQ.h"

struct ChromaConfig{
    unsigned int FS;
    double min;
    double max;
    unsigned int BPO;
    double CQThresh;
    MathUtilities::NormaliseType normalise;
};

class Chromagram 
{

public:	
    Chromagram( ChromaConfig Config );
    ~Chromagram();
	
    double* process( const double *data ); // time domain
    double* process( const double *real, const double *imag ); // frequency domain
    void unityNormalise( double* src );

    // Complex arithmetic
    double kabs( double real, double imag );
	
    // Results
    unsigned int getK() { return m_uK;}
    unsigned int getFrameSize() { return m_frameSize; }
    unsigned int getHopSize()   { return m_hopSize; }

private:
    int initialise( ChromaConfig Config );
    int deInitialise();

    Window<double> *m_window;
    double *m_windowbuf;
	
    double* m_chromadata;
    double m_FMin;
    double m_FMax;
    unsigned int m_BPO;
    unsigned int m_uK;

    MathUtilities::NormaliseType m_normalise;

    unsigned int m_frameSize;
    unsigned int m_hopSize;

    FFTReal* m_FFT;
    ConstantQ* m_ConstantQ;

    double* m_FFTRe;
    double* m_FFTIm;
    double* m_CQRe;
    double* m_CQIm;

    bool m_skGenerated;
};

#endif
