/***************************************************************************
                          probabilityvector.h  -  description
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

#ifndef PROBABILITYVECTOR_H
#define PROBABILITYVECTOR_H

#include "defs.h"

/**
  *@author Tue & Ken Haste Andersen
  */

class ProbabilityVector {
public: 
    ProbabilityVector(int size, int min);
    ~ProbabilityVector();
    void add(int dt, CSAMPLE weight);
    int maxIdx();
    void down();
private:
    /** Size of histogram */
    int size;
    /** Minimum index value considered when updating histogram in add() */
    int min;
    /** Pointer to histogram */
    CSAMPLE *hist;
    /** Maximum value of the histogram */
    CSAMPLE maxval;
    /** Index of the maximum value in the histogram */
    int maxvalIdx;

    
};

#endif
