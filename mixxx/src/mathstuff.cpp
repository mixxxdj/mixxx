#include "mathstuff.h"

// Defines modified Bessel function I_0(x) for any real x.
// From Numerical Recipes in C, sec.ed., p.237.
CSAMPLE besseli(CSAMPLE x)
{
    CSAMPLE ax, ans;
    CSAMPLE y;

    if ((ax=fabs(x)) < 3.75)
    {
        y = x/3.75;
        y *= y;
        ans = 1.0+y*(3.5156229+y*(3.0899424+y*(1.2067492
                                               +y*(0.2659732+y*(0.360768e-1+y*0.45813e-2)))));
    }
    else
    {
        y = 3.75/ax;
        ans=(exp(ax)/sqrt(ax))*(0.39894228+y*(0.1328592e-1
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
int invmatrix(CSAMPLE *m)
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
    CSAMPLE phi, ff, b, *s;
    s = new CSAMPLE[n+1];
    for (i=0; i<n; i++)
        s[i]=cof[i]=.0;
    s[n]= -x[0];
    for (i=1; i<=n; i++)
    {
        for (j=n-i; j<=n-1; j++)
            s[j] -= x[i]*s[j+1];
        s[n] -= x[i];
    }
    for (j=0; j<=n; j++)
    {
        phi=n+1;
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

// Fast arctan2 from http://www.dspguru.com/comp.dsp/tricks/alg/fxdatan2.htm
CSAMPLE arctan2(CSAMPLE y, CSAMPLE x)
{
    CSAMPLE r, angle;

    const CSAMPLE coeff_1 = pi/4;
    const CSAMPLE coeff_2 = 3*coeff_1;

    CSAMPLE abs_y = fabs(y)+1e-10;      // kludge to prevent 0/0 condition

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
        return(-angle);     // negate if in quad III or IV
    else
        return(angle);
}

CSAMPLE wndKaiser(CSAMPLE *wnd, int size, CSAMPLE beta)
{
    int m = size-1;
    CSAMPLE AFactor = 0.;

    CSAMPLE t = besseli(beta);
    for (int k=0; k<size; k++)
    {
        wnd[k] = besseli(2.*beta/m*sqrt(k*(m-k)))/t;
        AFactor += wnd[k];
    }
    return (2./AFactor);
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
