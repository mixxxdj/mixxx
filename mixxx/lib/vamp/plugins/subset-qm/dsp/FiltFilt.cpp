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

#include "FiltFilt.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FiltFilt::FiltFilt( FiltFiltConfig Config )
{
    m_filtScratchIn = NULL;
    m_filtScratchOut = NULL;
    m_ord = 0;
	
    initialise( Config );
}

FiltFilt::~FiltFilt()
{
    deInitialise();
}

void FiltFilt::initialise( FiltFiltConfig Config )
{
    m_ord = Config.ord;
    m_filterConfig.ord = Config.ord;
    m_filterConfig.ACoeffs = Config.ACoeffs;
    m_filterConfig.BCoeffs = Config.BCoeffs;
	
    m_filter = new Filter( m_filterConfig );
}

void FiltFilt::deInitialise()
{
    delete m_filter;
}


void FiltFilt::process(double *src, double *dst, unsigned int length)
{	
    unsigned int i;

    if (length == 0) return;

    unsigned int nFilt = m_ord + 1;
    unsigned int nFact = 3 * ( nFilt - 1);
    unsigned int nExt	= length + 2 * nFact;

    m_filtScratchIn = new double[ nExt ];
    m_filtScratchOut = new double[ nExt ];

	
    for( i = 0; i< nExt; i++ ) 
    {
	m_filtScratchIn[ i ] = 0.0;
	m_filtScratchOut[ i ] = 0.0;
    }

    // Edge transients reflection
    double sample0 = 2 * src[ 0 ];
    double sampleN = 2 * src[ length - 1 ];

    unsigned int index = 0;
    for( i = nFact; i > 0; i-- )
    {
	m_filtScratchIn[ index++ ] = sample0 - src[ i ];
    }
    index = 0;
    for( i = 0; i < nFact; i++ )
    {
	m_filtScratchIn[ (nExt - nFact) + index++ ] = sampleN - src[ (length - 2) - i ];
    }

    index = 0;
    for( i = 0; i < length; i++ )
    {
	m_filtScratchIn[ i + nFact ] = src[ i ];
    }
	
    ////////////////////////////////
    // Do  0Ph filtering
    m_filter->process( m_filtScratchIn, m_filtScratchOut, nExt);
	
    // reverse the series for FILTFILT 
    for ( i = 0; i < nExt; i++)
    { 
	m_filtScratchIn[ i ] = m_filtScratchOut[ nExt - i - 1];
    }

    // do FILTER again 
    m_filter->process( m_filtScratchIn, m_filtScratchOut, nExt);
	
    // reverse the series back 
    for ( i = 0; i < nExt; i++)
    {
	m_filtScratchIn[ i ] = m_filtScratchOut[ nExt - i - 1 ];
    }
    for ( i = 0;i < nExt; i++)
    {
	m_filtScratchOut[ i ] = m_filtScratchIn[ i ];
    }

    index = 0;
    for( i = 0; i < length; i++ )
    {
	dst[ index++ ] = m_filtScratchOut[ i + nFact ];
    }	

    delete [] m_filtScratchIn;
    delete [] m_filtScratchOut;

}

void FiltFilt::reset()
{

}
