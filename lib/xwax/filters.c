
#include <limits.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>

#include "filters.h"

/*
 * Computes an exponential moving average with the possibility to weight newly added
 * values with a factor alpha.
 */

void ema_init(struct ema_filter *filter, const double alpha)
{
    if (!filter) {
        errno = EINVAL;
        printf(__func__);
        return;
    }

    filter->alpha = alpha;
    filter->y_old = 0;
}

int ema(struct ema_filter *filter, const int x)
{
    if (!filter) {
        errno = EINVAL;
        printf(__func__);
        return -EINVAL;
    }

    int y = filter->alpha * x + (1 - filter->alpha) * filter->y_old;
    filter->y_old = y;

    return y;
}

/*
 * Computes a simple derivative, i.e. the slope of the input signal without gain compensation.
 */

void derivative_init(struct differentiator *filter)
{
    if (!filter) {
        errno = EINVAL;
        printf(__func__);
        return;
    }

    filter->x_old = 0;
}

int derivative(struct differentiator *filter, const int x)
{
    int y = x - filter->x_old;
    filter->x_old = x;

    return y;
}

/*
 * Computes the RMS value over a running sum.
 * The 1.0 > alpha > 0 determines the smoothness of the result:
 */

void rms_init(struct root_mean_square *filter, const float alpha)
{
    if (!filter) {
        errno = EINVAL;
        printf(__func__);
        return;
    }

    filter->squared_old = 0;
    filter->alpha = alpha;
}

int rms(struct root_mean_square *filter, const long long x)
{
    if (!filter) {
        errno = EINVAL;
        printf(__func__);
        return -EINVAL;
    }

    // Compute squared value
    unsigned long long squared = (unsigned long long) (x * x);

    // Apply EMA filter to squared values
    filter->squared_old = (1.0 - filter->alpha) * filter->squared_old + filter->alpha * squared;

    // Take square root at the end
    return (int) sqrt(filter->squared_old);
}
