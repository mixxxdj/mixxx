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
#include "maths/MathAliases.h"


struct ChromaConfig{
    unsigned int FS;
    fl_t min;
    fl_t max;
    unsigned int BPO;
    fl_t CQThresh;
    MathUtilities::NormaliseType normalise;
};

class Chromagram 
{

public:	
    Chromagram( ChromaConfig Config );
    ~Chromagram();
	
    fl_t* process( const fl_t *data ); // time domain
    fl_t* process( const fl_t *real, const fl_t *imag ); // frequency domain
    void unityNormalise( fl_t* src );

    // Complex arithmetic
    fl_t kabs( fl_t real, fl_t imag );
	
    // Results
    unsigned int getK() { return m_uK;}
    unsigned int getFrameSize() { return m_frameSize; }
    unsigned int getHopSize()   { return m_hopSize; }

private:
    int initialise( ChromaConfig Config );
    int deInitialise();

    Window<fl_t> *m_window;
    fl_t *m_windowbuf;
	
    fl_t* m_chromadata;
    fl_t m_FMin;
    fl_t m_FMax;
    unsigned int m_BPO;
    unsigned int m_uK;

    MathUtilities::NormaliseType m_normalise;

    unsigned int m_frameSize;
    unsigned int m_hopSize;

    FFTReal* m_FFT;
    ConstantQ* m_ConstantQ;

    fl_t* m_FFTRe;
    fl_t* m_FFTIm;
    fl_t* m_CQRe;
    fl_t* m_CQIm;

    bool m_skGenerated;
};

#endif
