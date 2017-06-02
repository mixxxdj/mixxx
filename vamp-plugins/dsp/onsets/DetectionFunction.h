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

#include "maths/MathUtilities.h"
#include "maths/MathAliases.h"
#include "dsp/phasevocoder/PhaseVocoder.h"
#include "base/Window.h"

#define DF_HFC (1)
#define DF_SPECDIFF (2)
#define DF_PHASEDEV (3)
#define DF_COMPLEXSD (4)
#define DF_BROADBAND (5)

struct DFConfig{
    unsigned int stepSize; // DF step in samples
    unsigned int frameLength; // DF analysis window - usually 2*step. Must be even!
    int DFType; // type of detection function ( see defines )
    fl_t dbRise; // only used for broadband df (and required for it)
    bool adaptiveWhitening; // perform adaptive whitening
    fl_t whiteningRelaxCoeff; // if < 0, a sensible default will be used
    fl_t whiteningFloor; // if < 0, a sensible default will be used
};

class DetectionFunction  
{
public:
    fl_t* getSpectrumMagnitude();
    DetectionFunction( DFConfig Config );
    virtual ~DetectionFunction();

    /**
     * Process a single time-domain frame of audio, provided as
     * frameLength samples.
     */
    fl_t processTimeDomain(const fl_t* samples);

    /**
     * Process a single frequency-domain frame, provided as
     * frameLength/2+1 real and imaginary component values.
     */
    fl_t processFrequencyDomain(const fl_t* reals, const fl_t* imags);

private:
    void whiten();
    fl_t runDF();

    fl_t HFC( unsigned int length, fl_t* src);
    fl_t specDiff( unsigned int length, fl_t* src);
    fl_t phaseDev(unsigned int length, fl_t *srcPhase);
    fl_t complexSD(unsigned int length, fl_t *srcMagnitude, fl_t *srcPhase);
    fl_t broadband(unsigned int length, fl_t *srcMagnitude);
	
private:
    void initialise( DFConfig Config );
    void deInitialise();

    int m_DFType;
    unsigned int m_dataLength;
    unsigned int m_halfLength;
    unsigned int m_stepSize;
    fl_t m_dbRise;
    bool m_whiten;
    fl_t m_whitenRelaxCoeff;
    fl_t m_whitenFloor;

    fl_t* m_magHistory;
    fl_t* m_phaseHistory;
    fl_t* m_phaseHistoryOld;
    fl_t* m_magPeaks;

    fl_t* m_windowed; // Array for windowed analysis frame
    fl_t* m_magnitude; // Magnitude of analysis frame ( frequency domain )
    fl_t* m_thetaAngle;// Phase of analysis frame ( frequency domain )
    fl_t* m_unwrapped; // Unwrapped phase of analysis frame

    Window<fl_t> *m_window;
    PhaseVocoder* m_phaseVoc;	// Phase Vocoder
};

#endif 
