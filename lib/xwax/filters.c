
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>

#include "filters.h"

/*
 * Initializes the exponential weighted moving average filter.
 */

void ewma_init(struct ewma_filter *f, const double alpha)
{
    if (!f) {
        errno = EINVAL;
        perror(__func__);
        return;
    }

    f->alpha = alpha;
    f->y_old = 0;
}

/*
 * Computes an exponential weighted moving average with the possibility to weight newly added
 * values with a factor alpha.
 */

int ewma(struct ewma_filter *f, const int x)
{
    if (!f) {
        errno = EINVAL;
        perror(__func__);
        return -EINVAL;
    }

    int y = f->alpha * x + (1 - f->alpha) * f->y_old;
    f->y_old = y;

    return y;
}

/*
 * Initializes the derivative filter.
 */

void derivative_init(struct differentiator *f)
{
    if (!f) {
        errno = EINVAL;
        perror(__func__);
        return;
    }

    f->x_old = 0;
}

/*
 * Computes a simple derivative, i.e. the slope of the input signal without gain compensation.
 */

int derivative(struct differentiator *f, const int x)
{
    int y = x - f->x_old;
    f->x_old = x;

    return y;
}

/*
 * Initializes the RMS filter
 */

void rms_init(struct root_mean_square *f, const float alpha)
{
    if (!f) {
        errno = EINVAL;
        perror(__func__);
        return;
    }

    f->squared_old = 0;
    f->alpha = alpha;
}

/*
 * Computes the RMS value over a running sum.
 * The 1.0 > alpha > 0 determines the smoothness of the result:
 */

int rms(struct root_mean_square *f, const int x)
{
    if (!f) {
        errno = EINVAL;
        perror(__func__);
        return -EINVAL;
    }

    /* Compute squared value */
    unsigned long long squared = (unsigned long long)x * (unsigned long long)x;

    /* Apply EMA filter to squared values */
    f->squared_old = (1.0 - f->alpha) * f->squared_old + f->alpha * squared;

    /* Take square root at the end */
    return (int)sqrt(f->squared_old);
}
