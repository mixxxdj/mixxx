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

#include "DetectionFunction.h"
#include <cstring>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DetectionFunction::DetectionFunction( DFConfig Config ) :
    m_window(0)
{
    m_magHistory = NULL;
    m_phaseHistory = NULL;
    m_phaseHistoryOld = NULL;
    m_magPeaks = NULL;

    initialise( Config );
}

DetectionFunction::~DetectionFunction()
{
    deInitialise();
}


void DetectionFunction::initialise( DFConfig Config )
{
    m_dataLength = Config.frameLength;
    m_halfLength = m_dataLength/2;

    m_DFType = Config.DFType;
    m_stepSize = Config.stepSize;

    m_whiten = Config.adaptiveWhitening;
    m_whitenRelaxCoeff = Config.whiteningRelaxCoeff;
    m_whitenFloor = Config.whiteningFloor;
    if (m_whitenRelaxCoeff < 0) m_whitenRelaxCoeff = 0.9997;
    if (m_whitenFloor < 0) m_whitenFloor = 0.01;

    m_magHistory = new double[ m_halfLength ];
    memset(m_magHistory,0, m_halfLength*sizeof(double));
		
    m_phaseHistory = new double[ m_halfLength ];
    memset(m_phaseHistory,0, m_halfLength*sizeof(double));

    m_phaseHistoryOld = new double[ m_halfLength ];
    memset(m_phaseHistoryOld,0, m_halfLength*sizeof(double));

    m_magPeaks = new double[ m_halfLength ];
    memset(m_magPeaks,0, m_halfLength*sizeof(double));

    // See note in process(const double *) below
    int actualLength = MathUtilities::previousPowerOfTwo(m_dataLength);
    m_phaseVoc = new PhaseVocoder(actualLength);

    m_DFWindowedFrame = new double[ m_dataLength ];
    m_magnitude = new double[ m_halfLength ];
    m_thetaAngle = new double[ m_halfLength ];

    m_window = new Window<double>(HanningWindow, m_dataLength);
}

void DetectionFunction::deInitialise()
{
    delete [] m_magHistory ;
    delete [] m_phaseHistory ;
    delete [] m_phaseHistoryOld ;
    delete [] m_magPeaks ;

    delete m_phaseVoc;

    delete [] m_DFWindowedFrame;
    delete [] m_magnitude;
    delete [] m_thetaAngle;

    delete m_window;
}

double DetectionFunction::process( const double *TDomain )
{
    m_window->cut( TDomain, m_DFWindowedFrame );

    // Our own FFT implementation supports power-of-two sizes only.
    // If we have to use this implementation (as opposed to the
    // version of process() below that operates on frequency domain
    // data directly), we will have to use the next smallest power of
    // two from the block size.  Results may vary accordingly!

    int actualLength = MathUtilities::previousPowerOfTwo(m_dataLength);

    if (actualLength != m_dataLength) {
        // Pre-fill mag and phase vectors with zero, as the FFT output
        // will not fill the arrays
        for (int i = actualLength/2; i < m_dataLength/2; ++i) {
            m_magnitude[i] = 0;
            m_thetaAngle[0] = 0;
        }
    }

    m_phaseVoc->process(m_DFWindowedFrame, m_magnitude, m_thetaAngle);

    if (m_whiten) whiten();

    return runDF();
}

double DetectionFunction::process( const double *magnitudes, const double *phases )
{
    for (size_t i = 0; i < m_halfLength; ++i) {
        m_magnitude[i] = magnitudes[i];
        m_thetaAngle[i] = phases[i];
    }

    if (m_whiten) whiten();

    return runDF();
}

void DetectionFunction::whiten()
{
    for (unsigned int i = 0; i < m_halfLength; ++i) {
        double m = m_magnitude[i];
        if (m < m_magPeaks[i]) {
            m = m + (m_magPeaks[i] - m) * m_whitenRelaxCoeff;
        }
        if (m < m_whitenFloor) m = m_whitenFloor;
        m_magPeaks[i] = m;
        m_magnitude[i] /= m;
    }
}

double DetectionFunction::runDF()
{
    double retVal = 0;

    switch( m_DFType )
    {
    case DF_HFC:
	retVal = HFC( m_halfLength, m_magnitude);
	break;
	
    case DF_SPECDIFF:
	retVal = specDiff( m_halfLength, m_magnitude);
	break;
	
    case DF_PHASEDEV:
	retVal = phaseDev( m_halfLength, m_thetaAngle);
	break;
	
    case DF_COMPLEXSD:
	retVal = complexSD( m_halfLength, m_magnitude, m_thetaAngle);
	break;

    case DF_BROADBAND:
        retVal = broadband( m_halfLength, m_magnitude);
        break;
    }
	
    return retVal;
}

double DetectionFunction::HFC(unsigned int length, double *src)
{
    unsigned int i;
    double val = 0;

    for( i = 0; i < length; i++)
    {
	val += src[ i ] * ( i + 1);
    }
    return val;
}

double DetectionFunction::specDiff(unsigned int length, double *src)
{
    unsigned int i;
    double val = 0.0;
    double temp = 0.0;
    double diff = 0.0;

    for( i = 0; i < length; i++)
    {
	temp = fabs( (src[ i ] * src[ i ]) - (m_magHistory[ i ] * m_magHistory[ i ]) );
		
	diff= sqrt(temp);

        // (See note in phaseDev below.)

        val += diff;

	m_magHistory[ i ] = src[ i ];
    }

    return val;
}


double DetectionFunction::phaseDev(unsigned int length, double *srcPhase)
{
    unsigned int i;
    double tmpPhase = 0;
    double tmpVal = 0;
    double val = 0;

    double dev = 0;

    for( i = 0; i < length; i++)
    {
	tmpPhase = (srcPhase[ i ]- 2*m_phaseHistory[ i ]+m_phaseHistoryOld[ i ]);
	dev = MathUtilities::princarg( tmpPhase );

        // A previous version of this code only counted the value here
        // if the magnitude exceeded 0.1.  My impression is that
        // doesn't greatly improve the results for "loud" music (so
        // long as the peak picker is reasonably sophisticated), but
        // does significantly damage its ability to work with quieter
        // music, so I'm removing it and counting the result always.
        // Same goes for the spectral difference measure above.
		
        tmpVal  = fabs(dev);
        val += tmpVal ;

	m_phaseHistoryOld[ i ] = m_phaseHistory[ i ] ;
	m_phaseHistory[ i ] = srcPhase[ i ];
    }
	
	
    return val;
}


double DetectionFunction::complexSD(unsigned int length, double *srcMagnitude, double *srcPhase)
{
    unsigned int i;
    double val = 0;
    double tmpPhase = 0;
    double tmpReal = 0;
    double tmpImag = 0;
   
    double dev = 0;
    ComplexData meas = ComplexData( 0, 0 );
    ComplexData j = ComplexData( 0, 1 );

    for( i = 0; i < length; i++)
    {
	tmpPhase = (srcPhase[ i ]- 2*m_phaseHistory[ i ]+m_phaseHistoryOld[ i ]);
	dev= MathUtilities::princarg( tmpPhase );
		
	meas = m_magHistory[i] - ( srcMagnitude[ i ] * exp( j * dev) );

	tmpReal = real( meas );
	tmpImag = imag( meas );

	val += sqrt( (tmpReal * tmpReal) + (tmpImag * tmpImag) );
		
	m_phaseHistoryOld[ i ] = m_phaseHistory[ i ] ;
	m_phaseHistory[ i ] = srcPhase[ i ];
	m_magHistory[ i ] = srcMagnitude[ i ];
    }

    return val;
}

double DetectionFunction::broadband(unsigned int length, double *src)
{
    double val = 0;
    for (unsigned int i = 0; i < length; ++i) {
        double sqrmag = src[i] * src[i];
        if (m_magHistory[i] > 0.0) {
            double diff = 10.0 * log10(sqrmag / m_magHistory[i]);
            if (diff > m_dbRise) val = val + 1;
        }
        m_magHistory[i] = sqrmag;
    }
    return val;
}        

double* DetectionFunction::getSpectrumMagnitude()
{
    return m_magnitude;
}

