#ifndef COMPLEX_H

#define COMPLEX_H

#include <stdint.h>

struct complex_q62 {
    int64_t re;
    int64_t im;
};

struct complex_q31 {
    int32_t re;
    int32_t im;
};

/* Complex multiplication in polar form
 *
 * e^ix * e^iy = cos(x)cos(y) - sin(x)sin(y) + cos(x)isin(y) + cos(y)isin(x)
 *			    {           real            } {           imag              }
 */

static inline struct complex_q62 complex_q31_mul(struct complex_q31 z0,
    struct complex_q31 z1)
{
    int64_t re = (int64_t)z0.re * z1.re - (int64_t)z0.im * z1.im;
    int64_t im = (int64_t)z0.re * z1.im + (int64_t)z1.re * z0.im;

    struct complex_q62 result = {re, im};

    return result;
}

static inline struct complex_q31 complex_q31_conj(struct complex_q31 z)
{
    z.im = -z.im;
    return z;
}

#endif /* end of include guard COMPLEX_H */
