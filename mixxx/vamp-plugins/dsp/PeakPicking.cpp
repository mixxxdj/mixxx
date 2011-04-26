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

#include "PeakPicking.h"
#include "Polyfit.h"

#include <iostream>
#include <cstring>


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PeakPicking::PeakPicking( PPickParams Config )
{
    m_workBuffer = NULL;
    initialise( Config );
}

PeakPicking::~PeakPicking()
{
    deInitialise();
}

void PeakPicking::initialise( PPickParams Config )
{
    m_DFLength = Config.length ;
    Qfilta = Config.QuadThresh.a ;
    Qfiltb = Config.QuadThresh.b ;
    Qfiltc = Config.QuadThresh.c ;
	
    m_DFProcessingParams.length = m_DFLength; 
    m_DFProcessingParams.LPOrd = Config.LPOrd; 
    m_DFProcessingParams.LPACoeffs = Config.LPACoeffs; 
    m_DFProcessingParams.LPBCoeffs = Config.LPBCoeffs; 
    m_DFProcessingParams.winPre  = Config.WinT.pre;
    m_DFProcessingParams.winPost = Config.WinT.post; 
    m_DFProcessingParams.AlphaNormParam = Config.alpha;
    m_DFProcessingParams.isMedianPositive = false;
	
    m_DFSmoothing = new DFProcess( m_DFProcessingParams );

    m_workBuffer = new double[ m_DFLength ];
    memset( m_workBuffer, 0, sizeof(double)*m_DFLength);
}

void PeakPicking::deInitialise()
{
    delete [] m_workBuffer;
    delete m_DFSmoothing;
    m_workBuffer = NULL;
}

void PeakPicking::process( double* src, unsigned int len, vector<int> &onsets )
{
    if (len < 4) return;

    vector <double> m_maxima;	

    // Signal conditioning 
    m_DFSmoothing->process( src, m_workBuffer );
	
    for( unsigned int u = 0; u < len; u++)
    {
	m_maxima.push_back( m_workBuffer[ u ] );		
    }
	
    quadEval( m_maxima, onsets );

    for( int b = 0; b <  m_maxima.size(); b++)
    {
	src[ b ] = m_maxima[ b ];
    }
}

int PeakPicking::quadEval( vector<double> &src, vector<int> &idx )
{
    unsigned int maxLength;

    vector <int> m_maxIndex;
    vector <int> m_onsetPosition;
	
    vector <double> m_maxFit;
    vector <double> m_poly;
    vector <double> m_err;

    double p;

    m_poly.push_back(0);
    m_poly.push_back(0);
    m_poly.push_back(0);

    for(  int t = -2; t < 3; t++)
    {
	m_err.push_back( (double)t );
    }
    for( unsigned int i = 2; i < src.size() - 2; i++)
    {
	if( (src[i] > src[i-1]) && (src[i] > src[i+1]) && (src[i] > 0) )
	{
//	    m_maxIndex.push_back(  i + 1 );
            m_maxIndex.push_back(i);
	}
    }

    maxLength = m_maxIndex.size();

    double selMax = 0;

    for( unsigned int j = 0; j < maxLength ; j++)
    {
        for (int k = -2; k <= 2; ++k)
	{
	    selMax = src[ m_maxIndex[j] + k ] ;
	    m_maxFit.push_back(selMax);			
	}

	p = TPolyFit::PolyFit2( m_err, m_maxFit, m_poly);

	double f = m_poly[0];
	double g = m_poly[1];
	double h = m_poly[2];

	int kk = m_poly.size();

	if (h < -Qfilta || f > Qfiltc)
	{
	    idx.push_back(m_maxIndex[j]);
	}
		
	m_maxFit.clear();
    }

    return 1;
}
