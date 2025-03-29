#include <assert.h>
#include <debug.h>
#include <errno.h>
#include <limits.h>
#include <math.h>

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

/*
 * Do Traktor-MK2-specific processing of the carrier wave
 *
 * Pushes samples into a delayline, computes the derivative, computes RMS
 * values and scales the derivative back up to the original signal's level.
 * Afterards the upscaled derivative can by processed by the pitch detection
 * algorithm.
 *
 * NOTE: Ideally the gain compensation should be done in the derivative and lowpass
 * filter structures by determining the amplitude response. I had this
 * implemented previously, but chose gain compensation by using the RMS value,
 * since it is easier to understand for developers not trained in signal
 * processing. Additionally it's nice to have the dB level at hand.
 *
 */

void mk2_process_carrier(struct timecoder *tc, signed int primary, signed int secondary)
{
    if (!tc) {
        errno = -EINVAL;
        perror(__func__);
        return;
    }

    /* Push the samples into the ringbuffer */
    delayline_push(&tc->primary.mk2.delayline, primary);
    delayline_push(&tc->secondary.mk2.delayline, secondary);

    /* Compute the discrete derivative */
    tc->primary.mk2.deriv = derivative(&tc->primary.mk2.differentiator,
            ema(&tc->primary.mk2.ema_filter, primary));
    tc->secondary.mk2.deriv = derivative(&tc->secondary.mk2.differentiator,
            ema(&tc->secondary.mk2.ema_filter, secondary));

    /* Compute the smoothed RMS value */
    tc->primary.mk2.rms = rms(&tc->primary.mk2.rms_filter, primary);
    tc->secondary.mk2.rms = rms(&tc->secondary.mk2.rms_filter, secondary);

    /* Compute the smoothed RMS value for the derivative */
    tc->primary.mk2.rms_deriv = rms(&tc->primary.mk2.rms_deriv_filter, tc->primary.mk2.deriv);
    tc->secondary.mk2.rms_deriv = rms(&tc->secondary.mk2.rms_deriv_filter, tc->secondary.mk2.deriv);

    /* Compute the gain compensation for the derivative*/
    tc->gain_compensation = (double)tc->secondary.mk2.rms / tc->secondary.mk2.rms_deriv;

    /* Without this limit pitch becomes too sensitive */
    if (tc->gain_compensation > 30.0)
        tc->gain_compensation = 30.0;

    tc->dB = 20 * log10((double)tc->secondary.mk2.rms / INT_MAX);

    /* Compute the scaled derivative */
    tc->primary.mk2.deriv_scaled = tc->primary.mk2.deriv * tc->gain_compensation;
    tc->secondary.mk2.deriv_scaled = tc->secondary.mk2.deriv * tc->gain_compensation;
}
