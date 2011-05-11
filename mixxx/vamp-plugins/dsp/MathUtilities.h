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

#ifndef MATHUTILITIES_H
#define MATHUTILITIES_H

// M_PI needs to be difined for Windows builds
#ifndef M_PI
#define M_PI    3.14159265358979323846f
#endif

#include <vector>

#include "nan-inf.h"

class MathUtilities  
{
public:	
    static double round( double x );

    static void	  getFrameMinMax( const double* data, unsigned int len,  double* min, double* max );

    static double mean( const double* src, unsigned int len );
    static double mean( const std::vector<double> &data,
                        unsigned int start, unsigned int count );
    static double sum( const double* src, unsigned int len );
    static double median( const double* src, unsigned int len );

    static double princarg( double ang );
    static double mod( double x, double y);

    static void	  getAlphaNorm(const double *data, unsigned int len, unsigned int alpha, double* ANorm);
    static double getAlphaNorm(const std::vector <double> &data, unsigned int alpha );

    static void   circShift( double* data, int length, int shift);

    static int	  getMax( double* data, unsigned int length, double* max = 0 );
    static int	  getMax( const std::vector<double> &data, double* max = 0 );
    static int    compareInt(const void * a, const void * b);

    enum NormaliseType {
        NormaliseNone,
        NormaliseUnitSum,
        NormaliseUnitMax
    };

    static void   normalise(double *data, int length,
                            NormaliseType n = NormaliseUnitMax);

    static void   normalise(std::vector<double> &data,
                            NormaliseType n = NormaliseUnitMax);

    // moving mean threshholding:
    static void adaptiveThreshold(std::vector<double> &data);

    static bool isPowerOfTwo(int x);
    static int nextPowerOfTwo(int x); // e.g. 1300 -> 2048, 2048 -> 2048
    static int previousPowerOfTwo(int x); // e.g. 1300 -> 1024, 2048 -> 2048
    static int nearestPowerOfTwo(int x); // e.g. 1300 -> 1024, 1700 -> 2048
};

#endif
