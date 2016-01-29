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

#ifndef FILTER_H
#define FILTER_H

#ifndef NULL
#define NULL 0
#endif

struct FilterConfig{
    unsigned int ord;
    double* ACoeffs;
    double* BCoeffs;
};

class Filter  
{
public:
    Filter( FilterConfig Config );
    virtual ~Filter();

    void reset();

    void process( double *src, double *dst, unsigned int length );
	

private:
    void initialise( FilterConfig Config );
    void deInitialise();

    unsigned int m_ord;

    double* m_inBuffer;
    double* m_outBuffer;

    double* m_ACoeffs;
    double* m_BCoeffs;
};

#endif
