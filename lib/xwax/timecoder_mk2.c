#include <assert.h>
#include <debug.h>
#include <errno.h>
#include <limits.h>

#include "timecoder_mk2.h"

#define REF_PEAKS_AVG 48 /* in wave cycles */

/*
 * Compute the LFSR bit (Traktor MK2 version)
 */

static inline mk2bits_t lfsr_mk2(mk2bits_t code, mk2bits_t taps)
{
    mk2bits_t taken;
    mk2bits_t xrs;

    taken = u128_and(code, taps);
    xrs = U128_ZERO;

    while (u128_neq(taken, U128_ZERO)) {
        xrs = u128_add(xrs, u128_and(taken, U128_ONE));
        taken = u128_rshift(taken, 1);
    }

    return u128_and(xrs, U128_ONE);
}

/*
 * Linear Feedback Shift Register in the forward direction. New values
 * are generated at the least-significant bit. (Traktor MK2 version)
 */

inline mk2bits_t fwd_mk2(mk2bits_t current, struct timecode_def *def)
{
    if (!def) {
        errno = -EINVAL;
        perror(__func__);
        return U128_ZERO;
    }

    mk2bits_t l;

    /* New bits are added at the MSB; shift right by one */
    l = lfsr_mk2(current, u128_or(def->taps_mk2,  U128_ONE));
    return u128_or(u128_rshift(current, 1), u128_lshift(l, (def->bits - 1)));
}

/*
 * Linear Feedback Shift Register in the reverse direction
 * (Traktor MK2 version)
 */

inline mk2bits_t rev_mk2(mk2bits_t current, struct timecode_def *def)
{
    if (!def) {
        errno = -EINVAL;
        perror(__func__);
        return U128_ZERO;
    }

    mk2bits_t l, mask;

    /* New bits are added at the LSB; shift left one and mask */
    mask = u128_sub(u128_lshift(U128_ONE,  def->bits), U128_ONE);
    l = lfsr_mk2(current,
         u128_or(u128_rshift(def->taps_mk2, 1),
         u128_lshift(U128_ONE, (def->bits - 1))));

    return u128_or(u128_and(u128_lshift(current, 1), mask), l);
}

/*
 * Where necessary, build the lookup table required for this timecode
 * (Traktor MK2 version)
 *
 * Return: -1 if not enough memory could be allocated, otherwise 0
 */

int build_lookup_mk2(struct timecode_def *def)
{
    if (!def) {
        errno = -EINVAL;
        perror(__func__);
        return -1;
    }

    unsigned int n;
    mk2bits_t current, next;

    if (def->lookup)
        return 0;

    fprintf(stderr, "Building LUT for %d bit %dHz timecode (%s)\n",
            def->bits, def->resolution, def->desc);

    if (lut_init_mk2(&def->lut_mk2, def->length) == -1)
        return -1;

    current = def->seed_mk2;

    for (n = 0; n < def->length; n++) {

        /* timecode must not wrap */
        assert(lut_lookup_mk2(&def->lut_mk2, &current) == (unsigned)-1);
        lut_push_mk2(&def->lut_mk2, &current);

        /* check symmetry of the lfsr functions */
        next = fwd_mk2(current, def);
        assert(u128_eq(rev_mk2(next, def), current));

        current = next;
    }

    def->lookup = true;

    return 0;
}
