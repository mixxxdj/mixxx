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

#ifndef DETECTIONFUNCTION_H
#define DETECTIONFUNCTION_H

#include "MathUtilities.h"
#include "MathAliases.h"
#include "PhaseVocoder.h"
#include "Window.h"

#define DF_HFC (1)
#define DF_SPECDIFF (2)
#define DF_PHASEDEV (3)
#define DF_COMPLEXSD (4)
#define DF_BROADBAND (5)

struct DFConfig{
    unsigned int stepSize; // DF step in samples
    unsigned int frameLength; // DF analysis window - usually 2*step
    int DFType; // type of detection function ( see defines )
    double dbRise; // only used for broadband df (and required for it)
    bool adaptiveWhitening; // perform adaptive whitening
    double whiteningRelaxCoeff; // if < 0, a sensible default will be used
    double whiteningFloor; // if < 0, a sensible default will be used
};

class DetectionFunction  
{
public:
    double* getSpectrumMagnitude();
    DetectionFunction( DFConfig Config );
    virtual ~DetectionFunction();
    double process( const double* TDomain );
    double process( const double* magnitudes, const double* phases );

private:
    void whiten();
    double runDF();

    double HFC( unsigned int length, double* src);
    double specDiff( unsigned int length, double* src);
    double phaseDev(unsigned int length, double *srcPhase);
    double complexSD(unsigned int length, double *srcMagnitude, double *srcPhase);
    double broadband(unsigned int length, double *srcMagnitude);
	
private:
    void initialise( DFConfig Config );
    void deInitialise();

    int m_DFType;
    unsigned int m_dataLength;
    unsigned int m_halfLength;
    unsigned int m_stepSize;
    double m_dbRise;
    bool m_whiten;
    double m_whitenRelaxCoeff;
    double m_whitenFloor;

    double* m_magHistory;
    double* m_phaseHistory;
    double* m_phaseHistoryOld;
    double* m_magPeaks;

    double* m_DFWindowedFrame; // Array for windowed analysis frame
    double* m_magnitude; // Magnitude of analysis frame ( frequency domain )
    double* m_thetaAngle;// Phase of analysis frame ( frequency domain )

    Window<double> *m_window;
    PhaseVocoder* m_phaseVoc;	// Phase Vocoder
};

#endif 
