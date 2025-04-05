#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stdio.h>

// Define the u128 struct using two 64-bit unsigned integers, with high part first.
typedef struct {
    uint64_t high;  // Most significant part
    uint64_t low;   // Least significant part
} u128;

// Macro to initialize a u128 at compile time.
#define U128(high, low) ((u128){ (high), (low) })
#define U128_ZERO       (u128) { 0ULL, 0ULL }
#define U128_ONE        (u128) { 0ULL, 1ULL }

static inline int u128_eq(u128 a, u128 b) 
{
    return (a.high == b.high) && (a.low == b.low);
}

// Not-equal comparison.
static inline int u128_neq(u128 a, u128 b) 
{
    return (a.high != b.high) || (a.low != b.low);
}

// Addition of two u128 values.
static inline u128 u128_add(u128 a, u128 b) 
{
    u128 result;

    uint64_t sum = a.low + b.low;
    uint64_t carry = (sum < a.low) ? 1 : 0;

    result.high = a.high + b.high + carry;
    result.low = sum;

    return result;
}

// Subtraction of two u128 values.
static inline u128 u128_sub(u128 a, u128 b) 
{
    u128 result;

    uint64_t diff = a.low - b.low;
    uint64_t borrow = (a.low < b.low) ? 1 : 0;

    result.high = a.high - b.high - borrow;
    result.low = diff;

    return result;
}

// Left shift by n bits.
static inline u128 u128_lshift(u128 a, uint32_t n) 
{
    u128 result;

    if (n >= 128) {
        result.high = 0LL;
        result.low = 0LL;
    } else if (n >= 64) {
        result.high = a.low << (n - 64);
        result.low = 0LL;
    } else {
        result.high = (a.high << n) | (a.low >> (64 - n));
        result.low = a.low << n;
    }

    return result;
}

// Right shift by n bits.
static inline u128 u128_rshift(u128 a, uint32_t n)
{
    u128 result;

    if (n >= 128) {
        result.high = 0ULL;
        result.low = 0ULL;
    } else if (n >= 64) {
        result.high = 0ULL;
        result.low = a.high >> (n - 64);
    } else {
        result.high = a.high >> n;
        result.low = (a.low >> n) | (a.high << (64 - n));
    }

    return result;
}

// Bitwise AND of two u128 values.
static inline u128 u128_and(u128 a, u128 b)
{
    u128 result;

    result.high = a.high & b.high;
    result.low = a.low & b.low;

    return result;
}

// Bitwise OR of two u128 values.
static inline u128 u128_or(u128 a, u128 b)
{
    u128 result;

    result.high = a.high | b.high;
    result.low = a.low | b.low;

    return result;
}

// Bitwise NOT (negation) of a u128 value.
static inline u128 u128_not(u128 a)
{
    u128 result;

    if (!a.low && !a.high) {
        result.high = 0ULL;
        result.low = 1ULL;
    } else {
        result.high = 0ULL;
        result.low = 0ULL;
    }

    return result;
}

// Print a u128 value in hexadecimal format (lowercase).
static inline void u128_print(u128 a) 
{
    printf("%016llx%016llx\n", (unsigned long long)a.high, (unsigned long long)a.low);
}

#endif /* end of include guard TYPES_H */

