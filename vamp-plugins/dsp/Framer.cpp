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

#include "Framer.h"
#include <math.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Framer::Framer()
{
    m_dataFrame = NULL;
    m_strideFrame = NULL;
}

Framer::~Framer()
{
    if( m_dataFrame != NULL )
	delete [] m_dataFrame;

    if( m_strideFrame != NULL )
	delete [] m_strideFrame;
}

void Framer::configure( unsigned int frameLength, unsigned int hop )
{
    m_frameLength = frameLength;
    m_stepSize = hop;

    resetCounters();

    if( m_dataFrame != NULL )
    {
	delete [] m_dataFrame;	
	m_dataFrame = NULL;
    }
    m_dataFrame = new double[ m_frameLength ];

    if( m_strideFrame != NULL )
    {
	delete [] m_strideFrame;	
	m_strideFrame = NULL;
    }
    m_strideFrame = new double[ m_stepSize ];
}

void Framer::getFrame(double *dst)
{

    if( (m_ulSrcIndex + ( m_frameLength) ) < m_ulSampleLen )
    {
	for( unsigned int u = 0; u < m_frameLength; u++)
	{
	    dst[ u ] = m_srcBuffer[ m_ulSrcIndex++ ]; 
	}	
	m_ulSrcIndex -= ( m_frameLength - m_stepSize );
    }
    else
    {
	unsigned int rem = (m_ulSampleLen - m_ulSrcIndex );
	unsigned int zero = m_frameLength - rem;

	for( unsigned int u = 0; u < rem; u++ )
	{
	    dst[ u ] = m_srcBuffer[ m_ulSrcIndex++ ];
	}
		
	for( unsigned int u = 0; u < zero; u++ )
	{
	    dst[ rem + u ] = 0;
	}

	m_ulSrcIndex -= (( rem - m_stepSize ) );
    }

    m_framesRead++;
}

void Framer::resetCounters()
{
    m_framesRead = 0;
    m_ulSrcIndex = 0;
}

unsigned int Framer::getMaxNoFrames()
{
    return m_maxFrames;
}

void Framer::setSource(double *src, unsigned int length)
{
    m_srcBuffer = src;
    m_ulSampleLen = length;

    m_maxFrames = (unsigned int)ceil( (double)m_ulSampleLen/(double)m_stepSize ) ;
}
