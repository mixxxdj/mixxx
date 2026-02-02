#ifndef COMPLEX_H

#define COMPLEX_H

#include <stdint.h>

/*
 * 32-bit signed fixed point datatype. The fixed point position depends on the usage context
 * and can be the ARM 1.31 format (Q0.31) and others.
 */

typedef int32_t q31_t;

/*
 * 64-bit signed fixed point datatype. The fixed point position depends on the usage context,
 * and can be the ARM 2.62 format (Q1.62) and others.
 */

typedef int64_t q63_t;

/*
 * A complex type in ARM Q format
 */

struct complex_q63 {
    q63_t re;
    q63_t im;
};

/*
 * A complex type in ARM Q format
 */

struct complex_q31 {
    q31_t re;
    q31_t im;
};

/*
 * Complex multiplication in polar form
 *
 * Imaginary number: i = sqrt(-1) --> i * i = -1
 *
 * e^(i*x) = cos(x) + i * sin(x)
 * e^(i*y) = cos(y) + i * sin(y)
 *
 * e^(i*x) * e^(i*y) = ( cos(x) + i * sin(x) ) * ( cos(y) + i * sin(y) )
 * e^(i*x) * e^(i*y) = cos(x) * cos(y) + cos(x) * i * sin(y) + cos(y) * i * sin(x) + i * sin(x) * i * sin(x)
 * e^(i*x) * e^(i*y) = cos(x) * cos(y) + cos(x) * i * sin(y) + cos(y) * i * sin(x) - sin(x) * sin(y)
 *
 * Everything that has an i, belongs to the imaginary part, so we reorder:
 *
 * e^(i*x) * e^(i*y) = cos(x) * cos(y) - sin(x) * sin(y) + cos(x) * i * sin(y) + cos(y) * i * sin(x)
 *                    {              real               } {                 imag                    }
 */

static inline struct complex_q63 complex_q31_mul(struct complex_q31 z0,
    struct complex_q31 z1)
{
    q63_t re = (q63_t)z0.re * z1.re - (q63_t)z0.im * z1.im;
    q63_t im = (q63_t)z0.re * z1.im + (q63_t)z1.re * z0.im;

    return (struct complex_q63) { re, im };
}

/*
 * Converts a complex_q31 into its complex conjugate
 */

static inline struct complex_q31 complex_q31_conj(struct complex_q31 z)
{
    z.im = -z.im;
    return z;
}

#endif /* end of include guard COMPLEX_H */
