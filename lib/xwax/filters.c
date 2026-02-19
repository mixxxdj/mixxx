#include "filters.h"
#include "fmatrix.h"

#include <errno.h>
#include <limits.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

double clamp_to_int32(double x)
{
    if (x > INT_MAX) {
        x = INT_MAX;
    } else if (x < INT_MIN) {
        x = INT_MIN;
    }

    return x;
}

void normalize_coeffs(double *coeffs, size_t N)
{
    double sum = 0.0;

    for (size_t i = 0; i < N; i++)
        sum += coeffs[i];

    for (size_t i = 0; i < N; i++)
        coeffs[i] /= sum;
}

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
    double squared = (double)x * (double)x;

    /* Apply EMA filter to squared values */
    f->squared_old = (1.0 - f->alpha) * f->squared_old + f->alpha * squared;

    /* Take square root at the end */
    return (int)lround(sqrt(f->squared_old));
}

/*
 * Computes the coefficients for a Savitzky-Golay filter at runtime
 * by using a Vandermonde matrix:
 *
 *      | (-M)^0 (-M)^1 (-M)^N |
 *      |    :      :      :   |
 *  A = |   0^0    0^1    0^N  |
 *      |    :      :      :   |
 *      |   M^0    M^1    M^N  |
 *
 * see https://c.mql5.com/forextsd/forum/147/sgfilter.pdf
 *
 * where M is the half-width of the sample window
 * and N is the order of the polynomial
 *
 *         N
 *  p(n) = Σ a_k * n^k
 *        k=0
 */

static double *savgol_gen_coeffs(size_t window_size, size_t polyorder)
{
    if (window_size % 2 == 0 || polyorder >= window_size) {
        errno = EINVAL;
        perror(__func__);
        return NULL;
    }

    struct fmatrix *A = NULL;
    struct fmatrix *AT = NULL;
    struct fmatrix *B = NULL;
    struct fmatrix *B_inv = NULL;
    struct fmatrix *H = NULL;

    size_t M = (window_size - 1) / 2;
    size_t N = window_size;
    size_t I = polyorder + 1;
    double *coeff = NULL;

    /* A: (N x I) */

    A = fmat_alloc(N, I);
    if (!A)
        goto out;

    /*
     * Fill the n x i Vandermonde matrix
     *
     *   a_n,i = n^i, -M <= n <= M
     *                 i = 0, 1, ..., N
     */

    for (size_t n = 0; n < N; n++) {
        int base = (int)n - (int)M; // May be negative

        for (size_t i = 0; i < I; i++) {
            fmat_set(A, n, i, pow((double)base, (double)i));
        }
    }

    /* AT = A^T */

    AT = fmat_trans(NULL, A);
    if (!AT)
        goto out;

    /* B = A^T x A */

    B = fmat_mul(NULL, AT, A);
    if (!B)
        goto out;

    /* B_inv */

    B_inv = fmat_inv(NULL, B);
    if (!B_inv)
        goto out;

    /* H = B_inv * AT */

    H = fmat_mul(NULL, B_inv, AT);
    if (!H)
        goto out;

    /* Allocate coefficient array */

    coeff = calloc(window_size, sizeof(double));
    if (!coeff)
        goto out;

    /* The coefficients can be obtained by extracting row 0 of H. */

    for (size_t i = 0; i < N; i++)
        coeff[i] = fmat_get(H, 0, i);

    normalize_coeffs(coeff, N);

    /* Cleanup */

out:
    fmat_free(A);
    fmat_free(AT);
    fmat_free(B);
    fmat_free(B_inv);
    fmat_free(H);

    return coeff;
}

/*
 * Creates a Savitzky-Golay filter at runtime given a windows size N
 * and filter order. The sample window size determines the delay of the filter.
 * The delay of the filter equals the half width M of the sample window.
 */

struct savitzky_golay *savgol_create(size_t window_size, size_t polyorder)
{
    if (window_size < 3 || window_size % 2 == 0 || polyorder >= window_size) {
        errno = EINVAL;
        if (polyorder >= window_size)
            fprintf(stderr,
                "%s: The filter order needs to be less than the window size\n",
                __func__);
        if (window_size % 2 == 0)
            fprintf(stderr, "%s: The window size N must be odd\n", __func__);
        goto error;
    }

    struct savitzky_golay *f = malloc(sizeof(*f));
    if (!f)
        goto error;

    f->window_size = window_size;
    f->polyorder = polyorder;
    f->M = (window_size - 1) / 2; /* Determine the half width */

    f->delayline = rb_alloc(window_size, sizeof(int));
    if (!f->delayline)
        goto error_rb_alloc;

    f->coeff = savgol_gen_coeffs(window_size, polyorder);
    if (!f->coeff)
        goto error_coeff;

    return f;

error_coeff:
    rb_free(f->delayline);
error_rb_alloc:
    free(f);
error:
    perror(__func__);

    return NULL;
}

/*
 * Destroys a Savitzky-Golay filter object
 */

void savgol_destroy(struct savitzky_golay *f)
{
    if (!f)
        return;

    rb_free(f->delayline);
    free(f->coeff);
    free(f);
}

/*
 * Applies a Savitzky-Golay filter to an input sample. The filter achieves
 * optimal smoothing of signal using least-squares polynomials.
 */

int savgol(struct savitzky_golay *f, int x)
{
    if (!f) {
        errno = EINVAL;
        perror(__func__);
        return 0;
    }

    rb_push(f->delayline, &x);

    double y = 0;
    int *sample = NULL; /* Sample pointer */

    for (size_t i = 0; i < f->window_size; i++) {
        sample = (int *)rb_at(f->delayline, i);
        y += f->coeff[i] * *sample;
    }

    /* Clamp as last security measure */

    y = clamp_to_int32(y);

    return (int)lround(y);
}

/*
 * Implements a simple highpass as rumble filter
 */

void rhpf_init(struct rumble_filter *f, unsigned int fs, double fc)
{
    if (!f) {
        errno = EINVAL;
        perror(__func__);
        return;
    }

    f->fs = (double)fs;
    f->fc = fc;

    double RC = 1.0 / (2.0 * M_PI * fc);
    double dt = 1.0 / fs;
    f->alpha = RC / (RC + dt);   /* from standard 1st order HP formula */

    f->x_old = 0.0;
    f->y_old = 0.0;
}

/*
 * Processes a sample in the rumble filter
 */

int rhpf_process(struct rumble_filter *f, int x)
{
    if (!f) {
        errno = EINVAL;
        perror(__func__);
        return 0;
    }

    double y = f->alpha * (f->y_old + x - f->x_old);

    f->x_old = x;
    f->y_old = y;

    return (int)lround(y);
}
