/***************************************************************************
                          probabilityvector.cpp  -  description
                             -------------------
    begin                : Fri Feb 14 2003
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
    email                : haste@diku.dk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "probabilityvector.h"

ProbabilityVector::ProbabilityVector(int _size, int _min)
{
    size = _size;
    min = _min;
    hist = new CSAMPLE[size];
    for (int i=0; i<size; i++)
        hist[i]=0.;

    maxval = 0.;
    maxvalIdx = 0;
}

ProbabilityVector::~ProbabilityVector()
{
    delete [] hist;
}

void ProbabilityVector::add(int dt, CSAMPLE weight)
{
    hist[dt-min] += weight;

    // Update maximum info
    if (hist[dt-min]>maxval)
    {
        maxval = hist[dt-min];
        maxvalIdx = dt-min;
    }
}

int ProbabilityVector::maxIdx()
{
    return min+maxvalIdx;
}

void ProbabilityVector::down()
{
    for (int i=0; i<size; i++)
        hist[i] *= 0.99;
}
