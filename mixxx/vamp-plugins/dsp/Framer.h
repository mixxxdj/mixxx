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

#ifndef FRAMER_H
#define FRAMER_H

//#include <io.h>
#include <fcntl.h>
#include <stdio.h>


class Framer  
{
public:
    void setSource( double* src, unsigned int length );
    unsigned int getMaxNoFrames();
    void getFrame( double* dst );
    void configure( unsigned int frameLength, unsigned int hop );
    Framer();
    virtual ~Framer();

    void resetCounters();

private:

    unsigned long	m_ulSampleLen;		// DataLength (samples)
    unsigned int	m_framesRead;		// Read Frames Index

    double*			m_srcBuffer;
    double*			m_dataFrame;		// Analysis Frame Buffer
    double*			m_strideFrame;		// Stride Frame Buffer
    unsigned int	m_frameLength;		// Analysis Frame Length
    unsigned int	m_stepSize;		// Analysis Frame Stride

    unsigned int	m_maxFrames;

    unsigned long	m_ulSrcIndex;
};

#endif
