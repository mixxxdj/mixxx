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

#ifndef TEMPOTRACK_H
#define TEMPOTRACK_H


#include <stdio.h>
#include <vector>

#include "dsp/signalconditioning/DFProcess.h"
#include "maths/Correlation.h"
#include "maths/MathAliases.h"
#include "dsp/signalconditioning/Framer.h"



using std::vector;

struct WinThresh
{
    unsigned int pre;
    unsigned int post;
};

struct TTParams
{
    unsigned int winLength; //Analysis window length
    unsigned int lagLength; //Lag & Stride size
    unsigned int alpha; //alpha-norm parameter
    unsigned int LPOrd; // low-pass Filter order
    fl_t* LPACoeffs; //low pass Filter den coefficients
    fl_t* LPBCoeffs; //low pass Filter num coefficients
    WinThresh WinT;//window size in frames for adaptive thresholding [pre post]:
};


class TempoTrack  
{
public:
    TempoTrack( TTParams Params );
    virtual ~TempoTrack();

    vector<int> process( vector <fl_t> DF, vector <fl_t> *tempoReturn = 0);

	
private:
    void initialise( TTParams Params );
    void deInitialise();

    int beatPredict( unsigned int FSP, fl_t alignment, fl_t period, unsigned int step);
    int phaseMM( fl_t* DF, fl_t* weighting, unsigned int winLength, fl_t period );
    void createPhaseExtractor( fl_t* Filter, unsigned int winLength,  fl_t period,  unsigned int fsp, unsigned int lastBeat );
    int findMeter( fl_t* ACF,  unsigned int len, fl_t period );
    void constDetect( fl_t* periodP, int currentIdx, int* flag );
    void stepDetect( fl_t* periodP, fl_t* periodG, int currentIdx, int* flag );
    void createCombFilter( fl_t* Filter, unsigned int winLength, unsigned int TSig, fl_t beatLag );
    fl_t tempoMM( fl_t* ACF, fl_t* weight, int sig );
	
    unsigned int m_dataLength;
    unsigned int m_winLength;
    unsigned int m_lagLength;

    fl_t		 m_rayparam;
    fl_t		 m_sigma;
    fl_t		 m_DFWVNnorm;

    vector<int>	 m_beats; // Vector of detected beats

    fl_t m_lockedTempo;

    fl_t* m_tempoScratch;
    fl_t* m_smoothRCF; // Smoothed Output of Comb Filterbank (m_tempoScratch)
	
    // Processing Buffers 
    fl_t* m_rawDFFrame; // Original Detection Function Analysis Frame
    fl_t* m_smoothDFFrame; // Smoothed Detection Function Analysis Frame
    fl_t* m_frameACF; // AutoCorrelation of Smoothed Detection Function 

    //Low Pass Coefficients for DF Smoothing
    fl_t* m_ACoeffs;
    fl_t* m_BCoeffs;
	
    // Objetcs/operators declaration
    Framer m_DFFramer;
    DFProcess* m_DFConditioning;
    Correlation m_correlator;
    // Config structure for DFProcess
    DFProcConfig m_DFPParams;

	// also want to smooth m_tempoScratch 
    DFProcess* m_RCFConditioning;
    // Config structure for RCFProcess
    DFProcConfig m_RCFPParams;



};

#endif
