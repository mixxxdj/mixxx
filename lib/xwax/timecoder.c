/*
 * Copyright (C) 2021 Mark Hills <mark@xwax.org>
 *
 * This file is part of "xwax".
 *
 * "xwax" is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License, version 3 as
 * published by the Free Software Foundation.
 *
 * "xwax" is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include <assert.h>
#include <limits.h>
#include <pitch.h>
#include <pitch_kalman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#define _USE_MATH_DEFINES
#include <math.h>

#include "complex.h"
#include "debug.h"
#include "filters.h"
#include "timecoder.h"
#include "timecoder_mk2.h"

#define ZERO_RC 0.001 /* time constant for zero/rumble filter */

#define REF_PEAKS_AVG 48 /* in wave cycles */

/* The number of correct bits which come in before the timecode is
 * declared valid. Set this too low, and risk the record skipping
 * around (often to blank areas of track) during scratching */

#define VALID_BITS 24

#define MONITOR_DECAY_EVERY 512 /* in samples */

#define SQ(x) ((x)*(x))
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*x))

/* Timecode definitions */

#define SWITCH_PHASE 0x1 /* tone phase difference of 270 (not 90) degrees */
#define SWITCH_PRIMARY 0x2 /* use left channel (not right) as primary */
#define SWITCH_POLARITY 0x4 /* read bit values in negative (not positive) */
#define TRAKTOR_MK2 0x8 /* use for Traktor MK2 timecode*/

static struct timecode_def timecodes[] = {
    {
     .name = "serato_2a",
     .desc = "Serato 2nd Ed., side A",
     .resolution = 1000,
     .bits = 20,
     .seed = 0x59017,
     .taps = 0x361e4,
     .length = 712000,
     .safe = 625000,
     .threshold = (128 << 16),
     },
    {
     .name = "serato_2b",
     .desc = "Serato 2nd Ed., side B",
     .resolution = 1000,
     .bits = 20,
     .seed = 0x8f3c6,
     .taps = 0x4f0d8,
     .length = 922000,
     .safe = 908000,
     .threshold = (128 << 16),
     },
    {
     .name = "serato_cd",
     .desc = "Serato CD",
     .resolution = 1000,
     .bits = 20,
     .seed = 0xd8b40,
     .taps = 0x34d54,
     .length = 950000,
     .safe = 890000,
     .threshold = (128 << 16),
     },
    {
     .name = "traktor_a",
     .desc = "Traktor Scratch, side A",
     .resolution = 2000,
     .flags = SWITCH_PRIMARY | SWITCH_POLARITY | SWITCH_PHASE,
     .bits = 23,
     .seed = 0x134503,
     .taps = 0x041040,
     .length = 1500000,
     .safe = 605000,
     .threshold = (128 << 16),
     },
    {
     .name = "traktor_b",
     .desc = "Traktor Scratch, side B",
     .resolution = 2000,
     .flags = SWITCH_PRIMARY | SWITCH_POLARITY | SWITCH_PHASE,
     .bits = 23,
     .seed = 0x32066c,
     .taps = 0x041040,
     .length = 2110000,
     .safe = 907000,
     .threshold = (128 << 16),
     },
    {
     .name = "traktor_mk2_a",
     .desc = "Traktor Scratch MK2, side A",
     .resolution = 2500,
     .flags = TRAKTOR_MK2,
     .bits = 110,
     .seed_mk2 = {
         .high = 0xc6007c63e,
         .low = 0x3fc00c60f8c1f00
     },
     .taps_mk2 = {
         .high = 0x400000000040,
         .low = 0x0000010800000001
     },
     .length = 1845000,
     .safe = 1795000,
     .threshold = (128 << 16),
     },
    {
     .name = "traktor_mk2_b",
     .desc = "Traktor Scratch MK2, side B",
     .resolution = 2500,
     .flags = TRAKTOR_MK2,
     .bits = 110,
     .seed_mk2 = {
         .high = 0x1ff9f00003,
         .low = 0xe73ff00f9fe0c7c1
     },
     .taps_mk2 = {
         .high = 0x400000000040,
         .low = 0x0000010800000001
     },
     .length = 2590000,
     .safe = 2540000,
     .threshold = (128 << 16),
     },
    {
     .name = "traktor_mk2_cd",
     .desc = "Traktor Scratch MK2, CD",
     .resolution = 3000,
     .flags = TRAKTOR_MK2,
     .bits = 110,
     .seed_mk2 = {
         .high = 0x7ce73,
         .low = 0xe0e0fff1fc1cf8c1
     },
     .taps_mk2 = {
         .high = 0x400000000000,
         .low = 0x1000010800000001
     },
     .length = 4500000,
     .safe = 4450000,
     .threshold = (128 << 16),
     },
    {
     .name = "mixvibes_v2",
     .desc = "MixVibes V2",
     .resolution = 1300,
     .flags = SWITCH_PHASE,
     .bits = 20,
     .seed = 0x22c90,
     .taps = 0x00008,
     .length = 950000,
     .safe = 655000,
     .threshold = (128 << 16),
     },
    {
     .name = "mixvibes_7inch",
     .desc = "MixVibes 7\"",
     .resolution = 1300,
     .flags = SWITCH_PHASE,
     .bits = 20,
     .seed = 0x22c90,
     .taps = 0x00008,
     .length = 312000,
     .safe = 238000,
     .threshold = (128 << 16),
     },
    {
     .name = "pioneer_a",
     .desc = "Pioneer RekordBox DVS Control Vinyl, side A",
     .resolution = 1000,
     .flags = SWITCH_POLARITY,
     .bits = 20,
     .seed = 0x78370,
     .taps = 0x7933a,
     .length = 635000,
     .safe = 614000,
     .threshold = (128 << 16),
     },
    {
     .name = "pioneer_b",
     .desc = "Pioneer RekordBox DVS Control Vinyl, side B",
     .resolution = 1000,
     .flags = SWITCH_POLARITY,
     .bits = 20,
     .seed = 0xf7012,
     .taps = 0x2ef1c,
     .length = 918500,
     .safe = 913000,
     .threshold = (128 << 16),
     },
};

/*
 * Calculate LFSR bit
 */

static inline bits_t lfsr(bits_t code, bits_t taps)
{
    bits_t taken;
    int xrs;

    taken = code & taps;
    xrs = 0;
    while (taken != 0x0) {
        xrs += taken & 0x1;
        taken >>= 1;
    }

    return xrs & 0x1;
}

/*
 * Linear Feedback Shift Register in the forward direction. New values
 * are generated at the least-significant bit.
 */

static inline bits_t fwd(bits_t current, struct timecode_def *def)
{
    bits_t l;

    /* New bits are added at the MSB; shift right by one */

    l = lfsr(current, def->taps | 0x1);
    return (current >> 1) | (l << (def->bits - 1));
}

/*
 * Linear Feedback Shift Register in the reverse direction
 */

static inline bits_t rev(bits_t current, struct timecode_def *def)
{
    bits_t l, mask;

    /* New bits are added at the LSB; shift left one and mask */

    mask = (1 << def->bits) - 1;
    l = lfsr(current, (def->taps >> 1) | (0x1 << (def->bits - 1)));
    return ((current << 1) & mask) | l;
}

/*
 * Where necessary, build the lookup table required for this timecode
 *
 * Return: -1 if not enough memory could be allocated, otherwise 0
 */

static int build_lookup(struct timecode_def *def)
{
    unsigned int n;
    bits_t current;

    if (def->lookup)
        return 0;

    fprintf(stderr, "Building LUT for %d bit %dHz timecode (%s)\n",
            def->bits, def->resolution, def->desc);

    if (lut_init(&def->lut, def->length) == -1)
	return -1;

    current = def->seed;

    for (n = 0; n < def->length; n++) {
        bits_t next;

        /* timecode must not wrap */
        assert(lut_lookup(&def->lut, current) == (unsigned)-1);
        lut_push(&def->lut, current);

        /* check symmetry of the lfsr functions */
        next = fwd(current, def);
        assert(rev(next, def) == current);

        current = next;
    }

    def->lookup = true;

    return 0;
}

/*
 * Find a timecode definition by name
 *
 * Return: pointer to timecode definition, or NULL if not available
 */

struct timecode_def* timecoder_find_definition(const char *name, const char *lut_dir_path)
{
    unsigned int n;

    for (n = 0; n < ARRAY_SIZE(timecodes); n++) {
        struct timecode_def *def = &timecodes[n];

        if (strcmp(def->name, name) != 0)
            continue;

        if (!def->lookup) {
            if (def->flags & TRAKTOR_MK2) {
                if (!lut_load_mk2(def, lut_dir_path))
                    return def;

                if (build_lookup_mk2(def) == -1)
                    return NULL;  /* error */

                if (lut_store_mk2(def, lut_dir_path)) {
                    timecoder_free_lookup();
                    fprintf(stderr, "Couldn't store LUT on disk\n");
                    return NULL;
                }
            } else {
                if (build_lookup(def) == -1)
                    return NULL;  /* error */
            }
        }
        return def;
    }

    return NULL;  /* not found */
}

/*
 * Free the timecoder lookup tables when they are no longer needed
 */

void timecoder_free_lookup(void) {
    unsigned int n;

    for (n = 0; n < ARRAY_SIZE(timecodes); n++) {
        struct timecode_def *def = &timecodes[n];

        if (def->flags & TRAKTOR_MK2) {
            if (def->lookup)
                lut_clear_mk2(&def->lut_mk2);
        } else {
            if (def->lookup)
                lut_clear(&def->lut);
        }
    }
}

/*
 * Initialise a subcode decoder for the Traktor MK2
 */

void mk2_subcode_init(struct mk2_subcode *sc)
{
    sc->valid_counter = 0;
    sc->avg_reading = INT_MAX/2;
    sc->avg_slope = INT_MAX/2;
    sc->bit = U128_ZERO;

    sc->readings = rb_alloc(3, sizeof(int));
    if (!sc->readings) {
        perror(__func__);
        return;
    }

    /* Initialise smoothing filters */
    ewma_init(&sc->ewma_reading, 0.01);
    ewma_init(&sc->ewma_slope, 0.01);
}

/*
 * Initialise filter values for one channel
 */

static void init_channel(struct timecode_def *def, struct timecoder_channel *ch,
    unsigned int sample_rate)
{
    ch->positive = false;
    ch->zero = 0;

    ch->deriv = INT_MAX/2;
    ch->rms = INT_MAX/2;
    ch->rms_deriv = 0;

    ch->delayline = rb_alloc(5, sizeof(int));
    if (!ch->delayline) {
        perror(__func__);
        return;
    }

    ch->delayline_deriv = rb_alloc(5, sizeof(int));
    if (!ch->delayline_deriv) {
        perror(__func__);
        return;
    }

    ewma_init_adaptive(&ch->ewma_filter, 5e-2, def->resolution, sample_rate);
    derivative_init(&ch->differentiator);

    rms_init(&ch->rms_filter, 1e-3);
    rms_init(&ch->rms_deriv_filter, 1e-3);

    size_t window = (size_t)ceil(sample_rate / def->resolution);
    if (window % 2 == 0)
        window++;
    ch->savgol_filter = savgol_create(window, 2);

    rhpf_init(&ch->rumble_filter, sample_rate, 50 * (def->resolution / 1000));
}

/*
 * Initialise a timecode decoder at the given reference speed
 *
 * Return: -1 if the timecoder could not be initialised, otherwise 0
 */

void timecoder_init(struct timecoder *tc, struct timecode_def *def,
                    double speed, unsigned int sample_rate, bool phono, bool pitch_estimator)
{
    assert(def != NULL);

    /* A definition contains a lookup table which can be shared
     * across multiple timecoders */

    assert(def->lookup);
    tc->def = def;
    tc->speed = speed;

    tc->dt = 1.0 / sample_rate;
    tc->sample_rate = sample_rate;
    tc->zero_alpha = tc->dt / (ZERO_RC + tc->dt);

    tc->threshold = tc->def->threshold;

    if (phono)
        tc->threshold >>= 5; /* approx -36dB */

    tc->forwards = 1;
    init_channel(tc->def, &tc->primary, sample_rate);
    init_channel(tc->def, &tc->secondary, sample_rate);

    tc->use_legacy_pitch_filter = pitch_estimator; /* Switch for pitch filter type */

    tc->dphi = 0.0;
    tc->freq = 0.0;
    tc->pitch = 0.0;

    if (tc->use_legacy_pitch_filter) {
        pitch_init(&tc->pitch_filter, tc->dt);
    } else {
        if (tc->def->flags & TRAKTOR_MK2) {
            pitch_kalman_init(&tc->pitch_kalman_filter, tc->dt,
                              KALMAN_COEFFS(1e-16, 1e-2), /* stable mode */
                              KALMAN_COEFFS(0.0135, 8e-6), /* scratch mode */
                              15, /* threshold */
                              false);
        } else {
            pitch_kalman_init(&tc->pitch_kalman_filter, tc->dt,
                              KALMAN_COEFFS(1e-16, 1e-2), /* stable mode */
                              KALMAN_COEFFS(0.0235, 1e-5), /* scratch mode */
                              5, /* threshold */
                              false);
        }
    }

    tc->ref_level = INT_MAX;
    tc->bitstream = 0;
    tc->timecode = 0;
    tc->valid_counter = 0;

    tc->ref_level = INT_MAX;
    tc->bitstream = 0;
    tc->timecode = 0;
    tc->valid_counter = 0;
    tc->timecode_ticker = 0;

    tc->mon = NULL;

    if (tc->def->flags & TRAKTOR_MK2) {
        mk2_subcode_init(&tc->upper_bitstream);
        mk2_subcode_init(&tc->lower_bitstream);
    }
}

/*
 * Clear resources associated with a timecode decoder
 */

void timecoder_clear(struct timecoder *tc)
{
    assert(tc->mon == NULL);

    rb_free(tc->primary.delayline);
    rb_free(tc->secondary.delayline);

    if (tc->def->flags & TRAKTOR_MK2) {
        rb_free(tc->upper_bitstream.readings);
        rb_free(tc->lower_bitstream.readings);
    }

    savgol_destroy(tc->primary.savgol_filter);
    savgol_destroy(tc->secondary.savgol_filter);
}

/*
 * Initialise a raster display of the incoming audio
 *
 * The monitor (otherwise known as 'scope' in the interface) is an x-y
 * display of the post-calibrated incoming audio.
 *
 * Return: -1 if not enough memory could be allocated, otherwise 0
 */

int timecoder_monitor_init(struct timecoder *tc, int size)
{
    assert(tc->mon == NULL);
    tc->mon_size = size;
    tc->mon = (unsigned char*)(malloc(SQ(tc->mon_size)));
    if (tc->mon == NULL) {
        perror("malloc");
        return -1;
    }
    memset(tc->mon, 0, SQ(tc->mon_size));
    tc->mon_counter = 0;
    return 0;
}

/*
 * Clear the monitor on the given timecoder
 */

void timecoder_monitor_clear(struct timecoder *tc)
{
    assert(tc->mon != NULL);
    free(tc->mon);
    tc->mon = NULL;
}

/*
 * Update channel information with axis-crossings
 */

static void detect_zero_crossing(struct timecoder_channel *ch,
                                 signed int v, double alpha,
                                 signed int threshold)
{
    ch->crossing_ticker++;

    ch->swapped = false;
    if (v > ch->zero + threshold && !ch->positive) {
        ch->swapped = true;
        ch->positive = true;
        ch->crossing_ticker = 0;
    } else if (v < ch->zero - threshold && ch->positive) {
        ch->swapped = true;
        ch->positive = false;
        ch->crossing_ticker = 0;
    }

    ch->zero += alpha * (v - ch->zero);
}

/*
 * Plot the given sample value in the x-y monitor
 */

static inline void update_monitor(struct timecoder *tc, signed int x, signed int y)
{
    int px, py, size, ref;

    if (!tc->mon)
        return;

    /* Only draw the monitor if the signal level is greater -40 dB */

    if (tc->dB < -40.0) {
        x = 0.0;
        y = 0.0;
    }

    size = tc->mon_size;
    ref = tc->ref_level;

    /* Decay the pixels already in the montior */

    if (++tc->mon_counter % MONITOR_DECAY_EVERY == 0) {
        int p;

        for (p = 0; p < SQ(size); p++) {
            if (tc->mon[p])
                tc->mon[p] = tc->mon[p] * 7 / 8;
        }
    }

    assert(ref > 0);

    /* ref_level is half the precision of signal level */
    px = size / 2 + (long long)x * size / ref / 8;
    py = size / 2 + (long long)y * size / ref / 8;

    if (px < 0 || px >= size || py < 0 || py >= size)
        return;

    tc->mon[py * size + px] = 0xff; /* white */
}

/*
 * Extract the bitstream from the sample value
 */

static void process_bitstream(struct timecoder *tc, signed int m)
{
    bits_t b;

    b = m > tc->ref_level;

    /* Add it to the bitstream, and work out what we were expecting
     * (timecode). */

    /* tc->bitstream is always in the order it is physically placed on
     * the vinyl, regardless of the direction. */

    if (tc->forwards) {
	tc->timecode = fwd(tc->timecode, tc->def);
	tc->bitstream = (tc->bitstream >> 1)
	    + (b << (tc->def->bits - 1));

    } else {
	bits_t mask;

	mask = ((1 << tc->def->bits) - 1);
	tc->timecode = rev(tc->timecode, tc->def);
	tc->bitstream = ((tc->bitstream << 1) & mask) + b;
    }

    if (tc->timecode == tc->bitstream)
	tc->valid_counter++;
    else {
	tc->timecode = tc->bitstream;
	tc->valid_counter = 0;
    }

    /* Take note of the last time we read a valid timecode */

    tc->timecode_ticker = 0;

    /* Adjust the reference level based on this new peak */

    tc->ref_level -= tc->ref_level / REF_PEAKS_AVG;
    tc->ref_level += m / REF_PEAKS_AVG;

    debug("%+6d zero, %+6d (ref %+6d)\t= %d%c (%5d)",
          tc->primary.zero,
          m, tc->ref_level,
	  b, tc->valid_counter == 0 ? 'x' : ' ',
	  tc->valid_counter);
}

/*
 * Computes the phase difference of a given sine-cosine pair using
 * complex number theory in Q0.31 for max efficiency.
 */

static inline double phase_difference(const int *cos0, const int *sin0,
                                      const int *cos1, const int *sin1)
{
    struct complex_q31 z0 = { .re = *cos0, .im = *sin0 };
    struct complex_q31 z1 = { .re = *cos1, .im = *sin1 };
    struct complex_q31 z1_conj = complex_q31_conj(z1);
    struct complex_q62 product = complex_q31_mul(z0, z1_conj);

    return atan2((double)product.im, (double)product.re);
}

/*
 * Various processing of the carrier wave needed for pitch detection.
 *
 * Pushes samples into a delayline, computes the derivative, filters it and
 * computes RMS values.
 * Afterards the upscaled derivative can by processed by the pitch detection
 * algorithm.
 */

void process_carrier(struct timecoder *tc, signed int primary,
    signed int secondary)
{
    if (!tc) {
        errno = -EINVAL;
        perror(__func__);
        return;
    }

    /* Push the samples into the ringbuffer */

    rb_push(tc->primary.delayline, &primary);
    rb_push(tc->secondary.delayline, &secondary);

    primary = rhpf_process(&tc->primary.rumble_filter, primary);
    secondary = rhpf_process(&tc->secondary.rumble_filter, secondary);

    /* Compute the discrete derivative */
    tc->primary.deriv = derivative(&tc->primary.differentiator,
        primary);
    tc->secondary.deriv = derivative(&tc->secondary.differentiator,
        secondary);

    tc->primary.deriv = ewma(&tc->primary.ewma_filter, tc->primary.deriv);
    tc->secondary.deriv = ewma(&tc->secondary.ewma_filter, tc->secondary.deriv);

    tc->primary.deriv_decoder = tc->primary.deriv;
    tc->secondary.deriv_decoder = tc->secondary.deriv;

    tc->primary.deriv = savgol(tc->primary.savgol_filter, tc->primary.deriv);
    tc->secondary.deriv = savgol(tc->secondary.savgol_filter, tc->secondary.deriv);

    /* Compute the smoothed RMS value */
    tc->primary.rms = rms(&tc->primary.rms_filter, primary);
    tc->secondary.rms = rms(&tc->secondary.rms_filter, secondary);

    /* Compute the smoothed RMS value for the derivative */
    tc->primary.rms_deriv =
        rms(&tc->primary.rms_deriv_filter, tc->primary.deriv);
    tc->secondary.rms_deriv =
        rms(&tc->secondary.rms_deriv_filter, tc->secondary.deriv);

    tc->dB = 20 * log10((double)tc->secondary.rms / INT_MAX);

    /* Push the derivative samples into the ringbuffer */
    rb_push(tc->primary.delayline_deriv, &tc->primary.deriv);
    rb_push(tc->secondary.delayline_deriv, &tc->secondary.deriv);
}

/*
 * Process a single sample from the incoming audio
 *
 * The two input signals (primary and secondary) are in the full range
 * of a signed int; ie. 32-bit signed.
 */

static void process_sample(struct timecoder *tc,
			   signed int primary, signed int secondary)
{
    if (tc->def->flags & TRAKTOR_MK2) {
        detect_zero_crossing(&tc->primary, tc->primary.deriv_decoder, tc->zero_alpha,
                tc->threshold);
        detect_zero_crossing(&tc->secondary, tc->secondary.deriv_decoder, tc->zero_alpha,
                tc->threshold);
    } else {
        detect_zero_crossing(&tc->primary, primary, tc->zero_alpha, tc->threshold);
        detect_zero_crossing(&tc->secondary, secondary, tc->zero_alpha, tc->threshold);
    }

    if (tc->dB > -40.0) {
        tc->dphi =
            phase_difference((int*)rb_at(tc->primary.delayline_deriv, 0),
                             (int*)rb_at(tc->secondary.delayline_deriv, 0),
                             (int*)rb_at(tc->primary.delayline_deriv, 1),
                             (int*)rb_at(tc->secondary.delayline_deriv, 1));

        double ddphi = 0.0; /* Derivative of the phase difference */

        if (tc->use_legacy_pitch_filter) {
            pitch_dt_observation(&tc->pitch_filter, tc->dphi);
            ddphi = tc->pitch_filter.v;
        } else {
            pitch_kalman_update(&tc->pitch_kalman_filter, tc->dphi);
            ddphi = tc->pitch_kalman_filter.Xk[1];
        }

        tc->freq = ddphi / (2.0 * M_PI);
        tc->pitch = (tc->freq / tc->def->resolution);
    } else {
        tc->freq = 0.0;
        tc->pitch = 0.0;
    }

    bool forwards;
    if (tc->freq >= 0.0)
        forwards = true;
    else
        forwards = false;

    if (tc->def->flags & SWITCH_PHASE) {
        tc->pitch = -tc->pitch;
        tc->freq = -tc->freq;
        forwards = !forwards;
    }

    if (forwards != tc->forwards) { /* direction has changed */
        tc->forwards = forwards;
        tc->valid_counter = 0;
    }

    /* If we have crossed the primary channel in the right polarity,
     * it's time to read off a timecode 0 or 1 value */

    if (tc->def->flags & TRAKTOR_MK2) {
        if (tc->secondary.swapped)
        {
            int reading = *(int *)rb_at(tc->secondary.delayline, 2);
            mk2_process_timecode(tc, reading);
        }
    } else {
        if (tc->secondary.swapped &&
           tc->primary.positive == ((tc->def->flags & SWITCH_POLARITY) == 0))
        {
            signed int m;

            /* scale to avoid clipping */
            m = abs(primary / 2 - tc->primary.zero / 2);
            process_bitstream(tc, m);
        }
    }

    tc->timecode_ticker++;
}

/*
 * Cycle to the next timecode definition which has a valid lookup
 *
 * Return: pointer to timecode definition
 */

static struct timecode_def* next_definition(struct timecode_def *def)
{
    assert(def != NULL);

    do {
        def++;

        if (def >= timecodes + ARRAY_SIZE(timecodes))
            def = timecodes;

    } while (!def->lookup);

    return def;
}

/*
 * Change the timecode definition to the next available
 */

void timecoder_cycle_definition(struct timecoder *tc)
{
    tc->def = next_definition(tc->def);
    tc->valid_counter = 0;
    tc->timecode_ticker = 0;
}

/*
 * Submit and decode a block of PCM audio data to the timecode decoder
 *
 * PCM data is in the full range of signed short; ie. 16-bit signed.
 */

void timecoder_submit(struct timecoder *tc, signed short *pcm, size_t npcm)
{
    while (npcm--) {
	signed int left, right, primary, secondary;

        left = pcm[0] << 16;
        right = pcm[1] << 16;

        if (tc->def->flags & SWITCH_PRIMARY) {
            primary = left;
            secondary = right;
        } else {
            primary = right;
            secondary = left;
        }

        process_carrier(tc, primary, secondary);

        if (tc->def->flags & TRAKTOR_MK2) {
            process_sample(tc, primary, secondary);

            /*
             * Display the derivative in the monitor. Since the signal is not
             * a perfect ring on the x-y-plane, but jumps up and down a bit,
             * it looks to small in the scope. Therefore a multiplication by
             * two is necessary.
             */

            update_monitor(tc, tc->primary.deriv * 2,
                    tc->secondary.deriv * 2);
        } else {
            process_sample(tc, primary, secondary);
            update_monitor(tc, left, right);
        }

        pcm += TIMECODER_CHANNELS;
    }
}

/*
 * Get the last-known position of the timecode
 *
 * If now data is available or if too few bits have been error
 * checked, then this counts as invalid. The last known position is
 * given along with the time elapsed since the position stamp was
 * read.
 *
 * Return: the known position of the timecode, or -1 if not known
 * Post: if when != NULL, *when is the elapsed time in seconds
 */

signed int timecoder_get_position(struct timecoder *tc, double *when)
{
    signed int r;

    if (tc->valid_counter <= VALID_BITS)
        return -1;

    if (tc->def->flags & TRAKTOR_MK2) {
        r = lut_lookup_mk2(&tc->def->lut_mk2, &tc->mk2_bitstream);
        if (r == -1)
            return -1;
    } else {
        r = lut_lookup(&tc->def->lut, tc->bitstream);
        if (r == -1)
            return -1;
    }

    if (r >= 0) {
        // normalize position to milliseconds, not timecode steps -- Owen
        r = (double)r * (1000.0 / ((double)tc->def->resolution * tc->speed));
    }

    if (when)
        *when = tc->timecode_ticker * tc->dt;

    return r;
}
