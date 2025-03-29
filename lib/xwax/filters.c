#include <math.h>

#include "filters.h"

/*
 * Computes an exponential moving average with the possibility to weight newly added
 * values with a factor alpha
 */
int ema(const int x, int *ema_old, const double alpha)
{
    int y = alpha * x + (1 - alpha) * *ema_old;
    *ema_old = y;
    return y;
}


/*
 * Computes a simple derivative, i.e. the slope of the input signal
 */
int derivative(const int x, int *x_old)
{
    int y = x - *x_old;
    *x_old = x;
    return y;
}

/*
 * Computes the RMS value over a running sum
 */
int rms(int x, unsigned long long *rms_old)
{
    static const float alpha = 1e-3; // EMA smoothing factor

    // Compute squared value
    unsigned long long squared = (unsigned long long) x * x;

    // Apply EMA filter to squared values
    *rms_old = (1.0 - alpha) * *rms_old + alpha * squared;

    // Take square root at the end
    return (int) sqrt(*rms_old);
}

double clamp(double x, double max)
{
    return x - ( (x > max) * (x - max) );
}
