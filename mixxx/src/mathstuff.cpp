/***************************************************************************
                          mathstuff.cpp  -  description
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

#include "mathstuff.h"

// Defines modified Bessel function I_0(x) for any real x.
// From Numerical Recipes in C, sec.ed., p.237.
CSAMPLE besseli(CSAMPLE x)
{
    CSAMPLE ax, ans;
    CSAMPLE y;

    if ((ax=fabs(x)) < 3.75)
    {
        y = x/3.75f;
        y *= y;
        ans = 1.0f+y*(3.5156229f+y*(3.0899424f+y*(1.2067492f
                                                  +y*(0.2659732f+y*(0.360768e-1+y*0.45813e-2)))));
    }
    else
    {
        y = 3.75f/ax;
        ans=(exp(ax)/sqrt(ax))*(0.39894228f+y*(0.1328592e-1
                                               +y*(0.225319e-2+y*(-0.157565e-2+y*(0.916281e-2
                                                                                  +y*(-0.2057706e-1+y*(0.2635537e-1+y*(-0.1647633e-1
                                                                                                                       +y*0.392377e-2))))))));
    }
    return ans;
}

// Returns the sing of the value X.
int sign(CSAMPLE x)
{
    if (fabs(x) == x)
        return 1;
    else
        return -1;
}

// Returns the inverse of the matrix [m[0] m[1]; m[1] m[2]] (call by name). If the
// operations is not possible -1 is returned, otherwise 0.
int invmatrix(CSAMPLE * m)
{
    CSAMPLE k = m[0]*m[2]-m[1]*m[1];
    CSAMPLE tmp;

    if (k == 0)
        return -1;
    else
    {
        tmp = m[2]/k;
        m[2] = m[0]/k;
        m[1] = -m[1]/k;
        m[0] = tmp;
        return 0;
    }
}

// Finds coefficients of an n order polynominal. From NR p.121.
void polcoe(CSAMPLE x[], CSAMPLE y[], int n, CSAMPLE cof[])
{
    int k,j,i;
    CSAMPLE phi, ff, b, * s;
    s = new CSAMPLE[n+1];
    for (i=0; i<n; i++)
        s[i]=cof[i]=.0;
    s[n]= -x[0];
    for (i=1; i<=n; ++i)
    {
        for (j=n-i; j<=n-1; j++)
            s[j] -= x[i]*s[j+1];
        s[n] -= x[i];
    }
    for (j=0; j<=n; ++j)
    {
        phi=n+1.f;
        for (k=n; k>=0; k--)
            phi=k*s[k]+x[j]*phi;
        ff=y[j]/phi;
        b=1.0;
        for (k=n; k>=0; k--)
        {
            cof[k] += b*ff;
            b = s[k]+x[j]*b;
        }
    }
    delete [] s;
}

// Calculates the value x mod 2pi. Returns a value between -pi and pi.
CSAMPLE mod2pi(CSAMPLE x)
{
    //  x += pi;
    x = x-floor(x/two_pi)*two_pi;
    CSAMPLE r = x;
    return(r);
}

#ifdef __WINDOWS__
// Rounds a CSAMPLE to nearest integer, and returns as int
int round(CSAMPLE x)
{
    double y = x;
    double z;
    double rest = modf(y,&z);
    int reti = (int)z;
    if (rest>=.5)
        reti++;
    else if (rest<-.5)
        reti--;

    return(reti);
}
#endif

// Fast arctan2 from http://www.dspguru.com/comp.dsp/tricks/alg/fxdatan2.htm
CSAMPLE arctan2(CSAMPLE y, CSAMPLE x)
{
    CSAMPLE r, angle;

    const CSAMPLE coeff_1 = pi/4;
    const CSAMPLE coeff_2 = 3*coeff_1;

    CSAMPLE abs_y = fabs(y)+1e-10f;      // kludge to prevent 0/0 condition

    if (x>=0)
    {
        r = (x - abs_y) / (x + abs_y);
        angle = coeff_1 - coeff_1 * r;
    }
    else
    {
        r = (x + abs_y) / (abs_y - x);
        angle = coeff_2 - coeff_1 * r;
    }

    if (y < 0)
        return(-angle); // negate if in quad III or IV
    else
        return(angle);
}

CSAMPLE wndKaiser(CSAMPLE * wnd, int size, CSAMPLE beta)
{
    int m = size-1;
    CSAMPLE AFactor = 0.;

    CSAMPLE t = besseli(beta);
    for (int k=0; k<size; ++k)
    {
        wnd[k] = besseli(2.*beta/m*sqrt((float)(k*(m-k))))/t;
        AFactor += wnd[k];
    }
    return (2.f/AFactor);
}

CSAMPLE wndKaiserSample(int size, CSAMPLE beta, int i)
{
    int m = size-1;
    CSAMPLE t = besseli(beta);
    return besseli(2.*beta/m*sqrt((float)(i*(m-i))))/t;
}

/*
 * // Calculate the derivative of the window in kaiser_wnd.
   // The result goes in kaiser_dwnd.
   void wndDwnd(CSAMPLE *wnd, CSAMPLE *dwnd, int size)
   {
    dwnd[0] = (wnd[1]-wnd[0])*SRATE;
    for (int k=0; k<size-1; k++)
        dwnd[k+1] = (wnd[k+1]-wnd[k])*SRATE;
   }
 */

double qip(CSAMPLE x, unsigned int n)
{
    double h = 1.;
    while (n)
    {
        if (n&1) h*=x;
        x*=x;
        n >>= 1;
    }
    return h;
}

bool even(long n)
{
//    if ((n/2) != (n+1)/2)
    if (n%2 != 0)
        return false;
    else
        return true;
}

float sigmoid_zero(double t, double max_t)
{
    //generates a sigmoid function but goes from 0 - t instead of
    //-t to +t.
    //furthermore we map t to a range of 0-5
    //center t around the half-way point
    double t1 = 10.0 * (t / max_t) - 5.0;
    return 1.0 / (1 + exp(0 - t1));
}

int nearestSuperiorPowerOfTwo(int v) {
    return (int)pow(2.0,ceil(log((double)v)/log(2.0)));
}
