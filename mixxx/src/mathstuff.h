/***************************************************************************
                          mathstuff.h  -  description
                             -------------------
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// Misc. math functios for Mixxx by Tue Haste Andersen.

#ifndef MATHSTUFF_H
#define MATHSTUFF_H

#include "defs.h"
#include <math.h>
#include <algorithm>

CSAMPLE besseli(CSAMPLE);
int sign(CSAMPLE);
int invmatrix(CSAMPLE *);
void polcoe(CSAMPLE x[], CSAMPLE y[], int n, CSAMPLE cof[]);
CSAMPLE mod2pi(CSAMPLE);
int round(CSAMPLE x);
CSAMPLE arctan2(CSAMPLE y, CSAMPLE x);
CSAMPLE wndKaiser(CSAMPLE *wnd, int size, CSAMPLE beta);
bool even(long n);
//void wndDwnd(CSAMPLE *wnd, CSAMPLE *dwnd, int size);
/** Compute pow(x,n) for positive integer n through repeated
  * squarings */
double qip(CSAMPLE x, unsigned int n);

static CSAMPLE pi     = acos(-1.0f);

#endif
