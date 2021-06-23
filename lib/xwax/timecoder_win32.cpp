/*
 * Copyright (C) 2013 Mark Hills <mark@xwax.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License version 2 for more details.
 *
 * You should have received a copy of the GNU General Public License
 * version 2 along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 */

extern "C" {

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif

#include "debug.h"
#include "timecoder.h"

#define ZERO_THRESHOLD (128 << 16)

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

static struct timecode_def timecodes[] = {
    {
        "serato_2a",
        "Serato 2nd Ed., side A",
        20,
        1000,
        0,
        0x59017,
        0x361e4,
        712000,
        625000,
        false
    },
    {
        "serato_2b",
        "Serato 2nd Ed., side B",
        20,
        1000,
        0,
        0x8f3c6,
        0x4f0d8, /* reverse of side A */
        922000,
        908000,
        false
    },
    {
        "serato_cd",
        "Serato CD",
        20,
        1000,
        0,
        0xd8b40,
        0x34d54,
        950000,
        890000,
        false
    },
    {
        "traktor_a",
        "Traktor Scratch, side A",
        23,
        2000,
        SWITCH_PRIMARY | SWITCH_POLARITY | SWITCH_PHASE,
        0x134503,
        0x041040,
        1500000,
        605000,
        false
    },
    {
        "traktor_b",
        "Traktor Scratch, side B",
        23,
        2000,
        SWITCH_PRIMARY | SWITCH_POLARITY | SWITCH_PHASE,
        0x32066c,
        0x041040, /* same as side A */
        2110000,
        907000,
        false
    },
    {
        "mixvibes_v2",
        "MixVibes V2",
        20,
        1300,
        SWITCH_PHASE,
        0x22c90,
        0x00008,
        950000,
        655000,
        false
    },
    {
        "mixvibes_7inch",
        "MixVibes 7\"",
        20,
        1300,
        SWITCH_PHASE,
        0x22c90,
        0x00008,
        312000,
        238000,
        false
    },
    {
        NULL
    }
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
        dassert(lut_lookup(&def->lut, current) == (unsigned)-1);
        lut_push(&def->lut, current);

        /* check symmetry of the lfsr functions */
        next = fwd(current, def);
        dassert(rev(next, def) == current);

        current = next;
    }

    def->lookup = true;

    return 0;
}

/*
 * Find a timecode definition by name
 *
 * Return: pointer to timecode definition, or NULL if not found
 */

struct timecode_def* timecoder_find_definition(const char *name)
{
    struct timecode_def *def, *end;

    def = &timecodes[0];
    end = def + ARRAY_SIZE(timecodes);

    for (;;) {
        if (!strcmp(def->name, name))
            break;

        def++;

        if (def == end)
            return NULL;
    }

    if (build_lookup(def) == -1)
        return NULL;

    return def;
}

/*
 * Free the timecoder lookup tables when they are no longer needed
 */

void timecoder_free_lookup(void) {
    struct timecode_def *def, *end;

    def = &timecodes[0];
    end = def + ARRAY_SIZE(timecodes);

    while (def < end) {
        if (def->lookup)
            lut_clear(&def->lut);
        def++;
    }
}

/*
 * Initialise filter values for one channel
 */

static void init_channel(struct timecoder_channel *ch)
{
    ch->positive = false;
    ch->zero = 0;
}

/*
 * Initialise a timecode decoder at the given reference speed
 *
 * Return: -1 if the timecoder could not be initialised, otherwise 0
 */

void timecoder_init(struct timecoder *tc, struct timecode_def *def,
                    double speed, unsigned int sample_rate, bool phono)
{
    assert(def != NULL);

    /* A definition contains a lookup table which can be shared
     * across multiple timecoders */

    assert(def->lookup);
    tc->def = def;
    tc->speed = speed;

    tc->dt = 1.0 / sample_rate;
    tc->zero_alpha = tc->dt / (ZERO_RC + tc->dt);
    tc->threshold = ZERO_THRESHOLD;
    if (phono)
        tc->threshold >>= 5; /* approx -36dB */

    tc->forwards = 1;
    init_channel(&tc->primary);
    init_channel(&tc->secondary);
    pitch_init(&tc->pitch, tc->dt);

    tc->ref_level = INT_MAX;
    tc->bitstream = 0;
    tc->timecode = 0;
    tc->valid_counter = 0;
    tc->timecode_ticker = 0;

    tc->mon = NULL;
}

/*
 * Clear resources associated with a timecode decoder
 */

void timecoder_clear(struct timecoder *tc)
{
    assert(tc->mon == NULL);
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

inline static void update_monitor(struct timecoder *tc, signed int x, signed int y)
{
    int px, py, size, ref;

    if (!tc->mon)
        return;

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

    /* ref_level is half the prevision of signal level */
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
 * Process a single sample from the incoming audio
 *
 * The two input signals (primary and secondary) are in the full range
 * of a signed int; ie. 32-bit signed.
 */

static void process_sample(struct timecoder *tc,
			   signed int primary, signed int secondary)
{
    detect_zero_crossing(&tc->primary, primary, tc->zero_alpha, tc->threshold);
    detect_zero_crossing(&tc->secondary, secondary, tc->zero_alpha, tc->threshold);

    /* If an axis has been crossed, use the direction of the crossing
     * to work out the direction of the vinyl */

    if (tc->primary.swapped || tc->secondary.swapped) {
        bool forwards;

        if (tc->primary.swapped) {
            forwards = (tc->primary.positive != tc->secondary.positive);
        } else {
            forwards = (tc->primary.positive == tc->secondary.positive);
        }

        if (tc->def->flags & SWITCH_PHASE)
	    forwards = !forwards;

        if (forwards != tc->forwards) { /* direction has changed */
            tc->forwards = forwards;
            tc->valid_counter = 0;
        }
    }

    /* If any axis has been crossed, register movement using the pitch
     * counters */

    if (!tc->primary.swapped && !tc->secondary.swapped)
	pitch_dt_observation(&tc->pitch, 0.0);
    else {
	double dx;

	dx = 1.0 / tc->def->resolution / 4;
	if (!tc->forwards)
	    dx = -dx;
	pitch_dt_observation(&tc->pitch, dx);
    }

    /* If we have crossed the primary channel in the right polarity,
     * it's time to read off a timecode 0 or 1 value */

    if (tc->secondary.swapped &&
       tc->primary.positive == ((tc->def->flags & SWITCH_POLARITY) == 0))
    {
        signed int m;

        /* scale to avoid clipping */
        m = abs(primary / 2 - tc->primary.zero / 2);
	process_bitstream(tc, m);
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

        if (def > timecodes + ARRAY_SIZE(timecodes))
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

	process_sample(tc, primary, secondary);
        update_monitor(tc, left, right);

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

    r = lut_lookup(&tc->def->lut, tc->bitstream);
    if (r == -1)
        return -1;

    if (r >= 0) {
        // normalize position to milliseconds, not timecode steps -- Owen
        r = (double)(r)
                * (1000.0
                   / ((double)(tc->def->resolution) * tc->speed));
    }

    if (when)
        *when = tc->timecode_ticker * tc->dt;

    return r;
}

}; // extern "C"
