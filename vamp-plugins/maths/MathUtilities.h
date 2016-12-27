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

#include <vector>

#include "nan-inf.h"
#include "MathAliases.h"

/**
 * Static helper functions for simple mathematical calculations.
 */
class MathUtilities  
{
public:	
    /**
     * Round x to the nearest integer.
     */
    static fl_t round( fl_t x );

    /**
     * Return through min and max pointers the highest and lowest
     * values in the given array of the given length.
     */
    static void	  getFrameMinMax( const fl_t* data, unsigned int len,  fl_t* min, fl_t* max );

    /**
     * Return the mean of the given array of the given length.
     */
    static fl_t mean( const fl_t* src, unsigned int len );

    /**
     * Return the mean of the subset of the given vector identified by
     * start and count.
     */
    static fl_t mean( const std::vector<fl_t> &data,
                        unsigned int start, unsigned int count );
    
    /**
     * Return the sum of the values in the given array of the given
     * length.
     */
    static fl_t sum( const fl_t* src, unsigned int len );

    /**
     * Return the median of the values in the given array of the given
     * length. If the array is even in length, the returned value will
     * be half-way between the two values adjacent to median.
     */
    static fl_t median( const fl_t* src, unsigned int len );

    /**
     * The principle argument function. Map the phase angle ang into
     * the range [-pi,pi).
     */
    static fl_t princarg( fl_t ang );

    /**
     * Floating-point division modulus: return x % y.
     */
    static fl_t mod( fl_t x, fl_t y);

    static void	  getAlphaNorm(const fl_t *data, unsigned int len, unsigned int alpha, fl_t* ANorm);
    static fl_t getAlphaNorm(const std::vector <fl_t> &data, unsigned int alpha );

    static void   circShift( fl_t* data, int length, int shift);

    static int	  getMax( fl_t* data, unsigned int length, fl_t* max = 0 );
    static int	  getMax( const std::vector<fl_t> &data, fl_t* max = 0 );
    static int    compareInt(const void * a, const void * b);

    enum NormaliseType {
        NormaliseNone,
        NormaliseUnitSum,
        NormaliseUnitMax
    };

    static void normalise(fl_t *data, int length,
                          NormaliseType n = NormaliseUnitMax);

    static void normalise(std::vector<fl_t> &data,
                          NormaliseType n = NormaliseUnitMax);

    /**
     * Threshold the input/output vector data against a moving-mean
     * average filter.
     */
    static void adaptiveThreshold(std::vector<fl_t> &data);

    /** 
     * Return true if x is 2^n for some integer n >= 0.
     */
    static bool isPowerOfTwo(int x);

    /**
     * Return the next higher integer power of two from x, e.g. 1300
     * -> 2048, 2048 -> 2048.
     */
    static int nextPowerOfTwo(int x);

    /**
     * Return the next lower integer power of two from x, e.g. 1300 ->
     * 1024, 2048 -> 2048.
     */
    static int previousPowerOfTwo(int x);

    /**
     * Return the nearest integer power of two to x, e.g. 1300 -> 1024,
     * 12 -> 16 (not 8; if two are equidistant, the higher is returned).
     */
    static int nearestPowerOfTwo(int x);

    /**
     * Return x!
     */
    static fl_t factorial(int x); // returns fl_t in case it is large

    /**
     * Return the greatest common divisor of natural numbers a and b.
     */
    static int gcd(int a, int b);
};

#endif
