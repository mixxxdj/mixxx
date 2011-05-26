/*
 * Copyright (C) 2010 Mark Hills <mark@pogo.org.uk>
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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif

#include "timecoder.h"

#define ZERO_THRESHOLD 128

#define ZERO_RC 0.001 /* time constant for zero/rumble filter */

#define REF_PEAKS_AVG 48 /* in wave cycles */

/* The number of correct bits which come in before the timecode 
 * is declared valid. Set this too low, and risk the record skipping around 
 * (often to blank areas of track) during scratching */

#define VALID_BITS 24

#define MONITOR_DECAY_EVERY 512 /* in samples */

#define SQ(x) ((x)*(x))


/* Timecode definitions */

#define SWITCH_PHASE 0x1 /* tone phase difference of 270 (not 90) degrees */
#define SWITCH_PRIMARY 0x2 /* use left channel (not right) as primary */
#define SWITCH_POLARITY 0x4 /* read bit values in negative (not positive) */


static struct timecode_def_t timecode_def[] = {
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
        905000,
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
        1214000,
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
        1814000,
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
        923000,
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
        310000,
        false
    },
    {
        NULL
    }
};


/* Linear Feeback Shift Register in the forward direction. New values
 * are generated at the least-significant bit. */

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


static inline bits_t fwd(bits_t current, struct timecode_def_t *def)
{
    bits_t l;

    /* New bits are added at the MSB; shift right by one */

    l = lfsr(current, def->taps | 0x1);
    return (current >> 1) | (l << (def->bits - 1));
}


static inline bits_t rev(bits_t current, struct timecode_def_t *def)
{
    bits_t l, mask;

    /* New bits are added at the LSB; shift left one and mask */

    mask = (1 << def->bits) - 1;
    l = lfsr(current, (def->taps >> 1) | (0x1 << (def->bits - 1)));
    return ((current << 1) & mask) | l;
}


static struct timecode_def_t* find_definition(const char *name)
{
    struct timecode_def_t *def;

    def = &timecode_def[0];
    while (def->name) {
        if (!strcmp(def->name, name))
            return def;
        def++;
    }
    return NULL;
}


/* Where necessary, build the lookup table required for this timecode */

static int build_lookup(struct timecode_def_t *def)
{
    unsigned int n;
    bits_t current, last;

    if (def->lookup)
        return 0;

    fprintf(stderr, "Building LUT for %d bit %dHz timecode (%s)\n",
            def->bits, def->resolution, def->desc);

    if (lut_init(&def->lut, def->length) == -1)
	return -1;

    current = def->seed;
    
    for (n = 0; n < def->length; n++) {
        /* timecode must not wrap */
        assert(lut_lookup(&def->lut, current) == (unsigned)-1);
        lut_push(&def->lut, current);
        last = current;
        current = fwd(current, def);
        assert(rev(current, def) == last);
    }

    def->lookup = true;
    
    return 0;    
}


/* Free the timecoder lookup tables when they are no longer needed */

void timecoder_free_lookup(void) {
    struct timecode_def_t *def;

    def = &timecode_def[0];
    while (def->name) {
        if (def->lookup)
            lut_clear(&def->lut);
        def++;
    }
}


static void init_channel(struct timecoder_channel_t *ch)
{
    ch->positive = 0;
    ch->zero = 0;
}


/* Initialise a timecode decoder at the given reference speed */

int timecoder_init(struct timecoder_t *tc, const char *def_name, double speed,
		   unsigned int sample_rate)
{
    /* A definition contains a lookup table which can be shared
     * across multiple timecoders */

    tc->def = find_definition(def_name);
    if (tc->def == NULL) {
        fprintf(stderr, "Timecode definition '%s' is not known.\n", def_name);
        return -1;
    }
    if (build_lookup(tc->def) == -1)
        return -1;
    tc->speed = speed;

    tc->dt = 1.0 / sample_rate;
    tc->zero_alpha = tc->dt / (ZERO_RC + tc->dt);

    tc->forwards = 1;
    init_channel(&tc->primary);
    init_channel(&tc->secondary);
    pitch_init(&tc->pitch, tc->dt);

    tc->ref_level = 32768.0;
    tc->bitstream = 0;
    tc->timecode = 0;
    tc->valid_counter = 0;
    tc->timecode_ticker = 0;

    tc->mon = NULL;

    return 0;
}


/* Clear a timecode decoder */

void timecoder_clear(struct timecoder_t *tc)
{
    timecoder_monitor_clear(tc);
}


/* The monitor (otherwise known as 'scope' in the interface) is the
 * display of the incoming audio. Initialise one for the given
 * timecoder */

int timecoder_monitor_init(struct timecoder_t *tc, int size)
{
    tc->mon_size = size;
    tc->mon = (unsigned char*)malloc(SQ(tc->mon_size));
    if (tc->mon == NULL) {
        perror("malloc");
        return -1;
    }
    memset(tc->mon, 0, SQ(tc->mon_size));
    tc->mon_counter = 0;
    return 0;
}


/* Clear the monitor on the given timecoder */

void timecoder_monitor_clear(struct timecoder_t *tc)
{
    if (tc->mon) {
        free(tc->mon);
        tc->mon = NULL;
    }
}


static void detect_zero_crossing(struct timecoder_channel_t *ch,
                                 signed int v, float alpha)
{
    ch->crossing_ticker++;

    ch->swapped = 0;
    if (v > ch->zero + ZERO_THRESHOLD && !ch->positive) {
        ch->swapped = 1;
        ch->positive = 1;
        ch->crossing_ticker = 0;
    } else if (v < ch->zero - ZERO_THRESHOLD && ch->positive) {
        ch->swapped = 1;
        ch->positive = 0;
        ch->crossing_ticker = 0;
    }
    
    ch->zero += alpha * (v - ch->zero);
}


/* Plot the given sample value in the monitor (scope) */

static void update_monitor(struct timecoder_t *tc, signed int x, signed int y)
{
    int px, py, p;
    float v, w;

    if (!tc->mon)
        return;

    /* Decay the pixels already in the montior */
        
    if (++tc->mon_counter % MONITOR_DECAY_EVERY == 0) {
        for (p = 0; p < SQ(tc->mon_size); p++) {
            if (tc->mon[p])
                tc->mon[p] = tc->mon[p] * 7 / 8;
        }
    }
        
    v = (float)x / tc->ref_level / 2;
    w = (float)y / tc->ref_level / 2;
        
    px = tc->mon_size / 2 + (v * tc->mon_size / 2);
    py = tc->mon_size / 2 + (w * tc->mon_size / 2);

    /* Set the pixel value to white */
            
    if (px > 0 && px < tc->mon_size && py > 0 && py < tc->mon_size)
        tc->mon[py * tc->mon_size + px] = 0xff;
}


/* Process a single bitstream reading */

static void process_bitstream(struct timecoder_t *tc, signed int m)
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

    tc->ref_level = (tc->ref_level * (REF_PEAKS_AVG - 1) + m) / REF_PEAKS_AVG;

#ifdef DEBUG_BITSTREAM
    fprintf(stderr, "%+6d zero, %+6d (ref %+6d)\t= %d%c (%5d)\n",
	    tc->primary.zero,
	    m,
	    tc->ref_level,
	    b,
	    tc->valid_counter == 0 ? 'x' : ' ',
	    tc->valid_counter);
#endif
}


/* Process a single sample from the incoming audio */

static void process_sample(struct timecoder_t *tc,
			   signed int primary, signed int secondary)
{
    signed int m; /* pcm sample, sum of two shorts */

    detect_zero_crossing(&tc->primary, primary, tc->zero_alpha);
    detect_zero_crossing(&tc->secondary, secondary, tc->zero_alpha);

    m = abs(primary - tc->primary.zero);

    /* If an axis has been crossed, use the direction of the crossing
     * to work out the direction of the vinyl */

    if (tc->primary.swapped) {
	tc->forwards = (tc->primary.positive != tc->secondary.positive);
	if (tc->def->flags & SWITCH_PHASE)
	    tc->forwards = !tc->forwards;
    } if (tc->secondary.swapped) {
	tc->forwards = (tc->primary.positive == tc->secondary.positive);
	if (tc->def->flags & SWITCH_PHASE)
	    tc->forwards = !tc->forwards;
    }

    /* If any axis has been crossed, register movement using the pitch
     * counters */

    if (!tc->primary.swapped && !tc->secondary.swapped)
	pitch_dt_observation(&tc->pitch, 0.0);
    else {
	float dx;

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
	process_bitstream(tc, m);
    }

    tc->timecode_ticker++;
}


/* Submit and decode a block of PCM audio data to the timecoder */

void timecoder_submit(struct timecoder_t *tc, const signed short *pcm, size_t npcm)
{
    while (npcm--) {
	signed int primary, secondary;

        if (tc->def->flags & SWITCH_PRIMARY) {
            primary = pcm[0];
            secondary = pcm[1];
        } else {
            primary = pcm[1];
            secondary = pcm[0];
        }

	process_sample(tc, primary, secondary);

        update_monitor(tc, pcm[0], pcm[1]);
        pcm += TIMECODER_CHANNELS;
    }
}


/* Return the known position in the timecode, or -1 if not known. If
 * two few bits have been error-checked, then this also counts as
 * invalid. If 'when' is given, return the time, in seconds since this
 * value was read. */

signed int timecoder_get_position(struct timecoder_t *tc, float *when)
{
    signed int r;

    if (tc->valid_counter > VALID_BITS) {
        r = lut_lookup(&tc->def->lut, tc->bitstream);

        if (r >= 0) {
        	//normalize position to milliseconds, not timecode steps -- Owen
        	r = (float)r * (1000.0 / (tc->def->resolution * tc->speed));
            if (when)
                *when = tc->timecode_ticker * tc->dt;
            return r;
        }
    }
    
    return -1;
}
