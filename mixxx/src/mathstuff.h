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
//void wndDwnd(CSAMPLE *wnd, CSAMPLE *dwnd, int size);

static CSAMPLE pi     = acos(-1.0);

#endif
