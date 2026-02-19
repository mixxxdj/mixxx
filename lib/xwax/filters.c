#include "filters.h"
#include "fmatrix.h"

#include <errno.h>
#include <limits.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

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

    size_t M = (window_size - 1) / 2;
    size_t N = window_size;
    size_t I = polyorder + 1;

    /* A: (N x I) */

    struct fmatrix *A = fmat_alloc(N, I);
    if (!A)
        goto error;

    /*
     * Fill the n x i Vandermonde matrix
     *
     *   a_n,i = n^i, -M <= n <= M
     *                 i = 0, 1, ..., N
     */

    for (size_t n = 0; n < N; n++) {
        int base = (int)n - (int)M;

        for (size_t i = 0; i < I; i++) {
            fmat_set(A, n, i, pow((double)base, (int)i));
        }
    }

    /* AT = A^T */

    struct fmatrix *AT = fmat_trans(NULL, A);
    if (!AT)
        goto error_at;

    /* B = A^T x A */

    struct fmatrix *B = fmat_mul(NULL, AT, A);
    if (!B)
        goto error_h;

    /* B_inv */

    struct fmatrix *B_inv = fmat_inv(NULL, B);
    if (!B_inv)
        goto error_b_inv;

    /* H = B_inv * AT */

    struct fmatrix *H = fmat_mul(NULL, B_inv, AT);
    if (!H)
        goto error_h;

    /* Allocate coefficient array */

    double *coeff = calloc(window_size, sizeof(double));
    if (!coeff)
        goto error_coeff;

    /*
     * The coefficients can be obtained by extracting row 0 of H.
     */

    for (size_t i = 0; i < I; i++)
        coeff[i] = fmat_get(H, 0, i);

    /* Cleanup */

    fmat_free(A);
    fmat_free(AT);
    fmat_free(B);
    fmat_free(B_inv);
    fmat_free(H);

    return coeff;

error_coeff:
    fmat_free(H);
error_h:
    fmat_free(B_inv);
error_b_inv:
    fmat_free(B);
error_b:
    fmat_free(AT);
error_at:
    fmat_free(A);
error:
    return NULL;
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

    if (f->delayline)
        rb_free(f->delayline);

    if (f->coeff)
        free(f->coeff);

    f->window_size = 0;
    f->polyorder = 0;
    f->M = 0;
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

    int y = 0;
    int *xh = NULL; /* Sample pointer */

    for (size_t i = 0; i < f->window_size; i++) {
        xh = (int *)rb_at(f->delayline, i);
        y += f->coeff[i] * *xh;
    }

    return y;
}
