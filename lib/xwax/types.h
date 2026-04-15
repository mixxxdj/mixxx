#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stdio.h>

/*
 * Define the u128 struct using two 64-bit unsigned integers, with high part first.
 */

typedef struct {
    uint64_t high;  /* Most significant part */
    uint64_t low;   /* Least significant part  */
} u128;

/*
 * Inline constructor for u128.
 * Works in all compilers, including MSVC.
 */

static inline u128 make_u128(uint64_t high, uint64_t low)
{
    u128 v;

    v.high = high;
    v.low = low;

    return v;
}

/*
 * Macro to preserve U128() syntax, but call the portable constructor.
 *
 * Used to be only the macro before, but MSVC doesn't like C99 compound
 * literals.
 */

#define U128(high, low) make_u128((high), (low))
#define U128_ZERO make_u128(0ULL, 0ULL)
#define U128_ONE  make_u128(0ULL, 1ULL)

static inline int u128_eq(u128 a, u128 b)
{
    return (a.high == b.high) && (a.low == b.low);
}

/*
 * Not-equal comparison.
 */

static inline int u128_neq(u128 a, u128 b)
{
    return (a.high != b.high) || (a.low != b.low);
}

/*
 * Addition of two u128 values.
 */

static inline u128 u128_add(u128 a, u128 b)
{
    uint64_t sum = a.low + b.low;
    uint64_t carry = (sum < a.low) ? 1 : 0;

    return U128(a.high + b.high + carry, sum);
}

/*
 * Subtraction of two u128 values.
 */

static inline u128 u128_sub(u128 a, u128 b)
{
    uint64_t diff = a.low - b.low;
    uint64_t borrow = (a.low < b.low) ? 1 : 0;

    return U128(a.high - b.high - borrow, diff);
}

/*
 * Left shift by n bits.
 */

static inline u128 u128_lshift(u128 a, uint32_t n)
{
    if (n >= 128)
        return U128_ZERO;
    else if (n >= 64)
        return U128(a.low << (n - 64), 0ULL);
    else
        return U128((a.high << n) | (a.low >> (64 - n)), a.low << n);
}

/*
 * Right shift by n bits.
 */

static inline u128 u128_rshift(u128 a, uint32_t n)
{
    if (n >= 128)
        return U128_ZERO;
    else if (n >= 64)
        return U128(0ULL, a.high >> (n - 64));
    else
        return U128(a.high >> n, (a.low >> n) | (a.high << (64 - n)));
}

/*
 * Bitwise AND of two u128 values.
 */

static inline u128 u128_and(u128 a, u128 b)
{
    return U128(a.high & b.high, a.low & b.low);
}

/*
 * Bitwise OR of two u128 values.
 */

static inline u128 u128_or(u128 a, u128 b)
{
    return U128(a.high | b.high, a.low | b.low);
}

/*
 * Logical NOT (negation) of a u128 value.
 */

static inline u128 u128_not(u128 a)
{
    if (!a.low && !a.high)
        return U128(0ULL, 1ULL);
    else
        return U128(0ULL, 0ULL);
}

/*
 * Print a u128 value in hexadecimal format (lowercase).
 */

static inline void u128_print(u128 a)
{
    printf("%016llx%016llx\n", (unsigned long long)a.high, (unsigned long long)a.low);
}

#endif /* end of include guard TYPES_H */

