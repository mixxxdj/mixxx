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

// PeakPicking.h: interface for the PeakPicking class.
//
//////////////////////////////////////////////////////////////////////

#ifndef PEAKPICKING_H
#define PEAKPICKING_H

#include "MathUtilities.h"
#include "MathAliases.h"
#include "DFProcess.h"


struct PPWinThresh
{
    unsigned int pre;
    unsigned int  post;
};

struct QFitThresh
{
    double a;
    double b;
    double c;
};

struct PPickParams
{
    unsigned int length; //Detection FunctionLength
    double tau; // time resolution of the detection function:
    unsigned int alpha; //alpha-norm parameter
    double cutoff;//low-pass Filter cutoff freq
    unsigned int LPOrd; // low-pass Filter order
    double* LPACoeffs; //low pass Filter den coefficients
    double* LPBCoeffs; //low pass Filter num coefficients
    PPWinThresh WinT;//window size in frames for adaptive thresholding [pre post]:
    QFitThresh QuadThresh;
};

class PeakPicking  
{
public:
    PeakPicking( PPickParams Config );
    virtual ~PeakPicking();
	
    void process( double* src, unsigned int len, vector<int> &onsets  );


private:
    void initialise( PPickParams Config  );
    void deInitialise();
    int  quadEval( vector<double> &src, vector<int> &idx );
	
    DFProcConfig m_DFProcessingParams;

    unsigned int m_DFLength ;
    double Qfilta ;
    double Qfiltb;
    double Qfiltc;


    double* m_workBuffer;
	
    DFProcess*	m_DFSmoothing;
};

#endif
