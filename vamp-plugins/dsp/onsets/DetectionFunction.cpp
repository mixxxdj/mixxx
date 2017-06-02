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
    m_halfLength = m_dataLength/2 + 1;

    m_DFType = Config.DFType;
    m_stepSize = Config.stepSize;
    m_dbRise = Config.dbRise;

    m_whiten = Config.adaptiveWhitening;
    m_whitenRelaxCoeff = Config.whiteningRelaxCoeff;
    m_whitenFloor = Config.whiteningFloor;
    if (m_whitenRelaxCoeff < 0) m_whitenRelaxCoeff = 0.9997;
    if (m_whitenFloor < 0) m_whitenFloor = 0.01;

    m_magHistory = new fl_t[ m_halfLength ];
    memset(m_magHistory,0, m_halfLength*sizeof(fl_t));
		
    m_phaseHistory = new fl_t[ m_halfLength ];
    memset(m_phaseHistory,0, m_halfLength*sizeof(fl_t));

    m_phaseHistoryOld = new fl_t[ m_halfLength ];
    memset(m_phaseHistoryOld,0, m_halfLength*sizeof(fl_t));

    m_magPeaks = new fl_t[ m_halfLength ];
    memset(m_magPeaks,0, m_halfLength*sizeof(fl_t));

    m_phaseVoc = new PhaseVocoder(m_dataLength, m_stepSize);

    m_magnitude = new fl_t[ m_halfLength ];
    m_thetaAngle = new fl_t[ m_halfLength ];
    m_unwrapped = new fl_t[ m_halfLength ];

    m_window = new Window<fl_t>(HanningWindow, m_dataLength);
    m_windowed = new fl_t[ m_dataLength ];
}

void DetectionFunction::deInitialise()
{
    delete [] m_magHistory ;
    delete [] m_phaseHistory ;
    delete [] m_phaseHistoryOld ;
    delete [] m_magPeaks ;

    delete m_phaseVoc;

    delete [] m_magnitude;
    delete [] m_thetaAngle;
    delete [] m_windowed;
    delete [] m_unwrapped;

    delete m_window;
}

fl_t DetectionFunction::processTimeDomain(const fl_t *samples)
{
    m_window->cut(samples, m_windowed);

    m_phaseVoc->processTimeDomain(m_windowed, 
                                  m_magnitude, m_thetaAngle, m_unwrapped);

    if (m_whiten) whiten();

    return runDF();
}

fl_t DetectionFunction::processFrequencyDomain(const fl_t *reals,
                                                 const fl_t *imags)
{
    m_phaseVoc->processFrequencyDomain(reals, imags,
                                       m_magnitude, m_thetaAngle, m_unwrapped);

    if (m_whiten) whiten();

    return runDF();
}

void DetectionFunction::whiten()
{
    for (unsigned int i = 0; i < m_halfLength; ++i) {
        fl_t m = m_magnitude[i];
        if (m < m_magPeaks[i]) {
            m = m + (m_magPeaks[i] - m) * m_whitenRelaxCoeff;
        }
        if (m < m_whitenFloor) m = m_whitenFloor;
        m_magPeaks[i] = m;
        m_magnitude[i] /= m;
    }
}

fl_t DetectionFunction::runDF()
{
    fl_t retVal = 0;

    switch( m_DFType )
    {
    case DF_HFC:
	retVal = HFC( m_halfLength, m_magnitude);
	break;
	
    case DF_SPECDIFF:
	retVal = specDiff( m_halfLength, m_magnitude);
	break;
	
    case DF_PHASEDEV:
        // Using the instantaneous phases here actually provides the
        // same results (for these calculations) as if we had used
        // unwrapped phases, but without the possible accumulation of
        // phase error over time
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

fl_t DetectionFunction::HFC(unsigned int length, fl_t *src)
{
    unsigned int i;
    fl_t val = 0;

    for( i = 0; i < length; i++)
    {
	val += src[ i ] * ( i + 1);
    }
    return val;
}

fl_t DetectionFunction::specDiff(unsigned int length, fl_t *src)
{
    unsigned int i;
    fl_t val = 0.0;
    fl_t temp = 0.0;
    fl_t diff = 0.0;

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


fl_t DetectionFunction::phaseDev(unsigned int length, fl_t *srcPhase)
{
    unsigned int i;
    fl_t tmpPhase = 0;
    fl_t tmpVal = 0;
    fl_t val = 0;

    fl_t dev = 0;

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


fl_t DetectionFunction::complexSD(unsigned int length, fl_t *srcMagnitude, fl_t *srcPhase)
{
    unsigned int i;
    fl_t val = 0;
    fl_t tmpPhase = 0;
    fl_t tmpReal = 0;
    fl_t tmpImag = 0;
   
    fl_t dev = 0;
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

fl_t DetectionFunction::broadband(unsigned int length, fl_t *src)
{
    fl_t val = 0;
    for (unsigned int i = 0; i < length; ++i) {
        fl_t sqrmag = src[i] * src[i];
        if (m_magHistory[i] > 0.0) {
            fl_t diff = 10.0 * log10(sqrmag / m_magHistory[i]);
            if (diff > m_dbRise) val = val + 1;
        }
        m_magHistory[i] = sqrmag;
    }
    return val;
}        

fl_t* DetectionFunction::getSpectrumMagnitude()
{
    return m_magnitude;
}

