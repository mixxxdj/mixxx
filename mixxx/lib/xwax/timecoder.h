/*
 * Copyright (C) 2012 Mark Hills <mark@pogo.org.uk>
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

#ifndef TIMECODER_H
#define TIMECODER_H

#ifndef _MSC_VER
#include <stdbool.h>
#endif

#include "lut.h"
#include "pitch.h"

#define TIMECODER_CHANNELS 2

typedef unsigned int bits_t;

struct timecode_def {
    char *name, *desc;
    int bits, /* number of bits in string */
        resolution, /* wave cycles per second */
        flags;
    bits_t seed, /* LFSR value at timecode zero */
        taps; /* central LFSR taps, excluding end taps */
    unsigned int length, /* in cycles */
        safe; /* last 'safe' timecode number (for auto disconnect) */
    bool lookup; /* true if lut has been generated */
    struct lut lut;
};

struct timecoder_channel {
    bool positive, /* wave is in positive part of cycle */
	swapped; /* wave recently swapped polarity */
    signed int zero;
    unsigned int crossing_ticker; /* samples since we last crossed zero */
};

struct timecoder {
    struct timecode_def *def;
    double speed;

    /* Precomputed values */

    double dt, zero_alpha;

    /* Pitch information */

    bool forwards;
    struct timecoder_channel primary, secondary;
    struct pitch pitch;

    /* Numerical timecode */

    signed int ref_level;
    bits_t bitstream, /* actual bits from the record */
        timecode; /* corrected timecode */
    unsigned int valid_counter, /* number of successful error checks */
        timecode_ticker; /* samples since valid timecode was read */

    /* Feedback */

    unsigned char *mon; /* x-y array */
    int mon_size, mon_counter;
};

struct timecode_def* timecoder_find_definition(const char *name);
void timecoder_free_lookup(void);

void timecoder_init(struct timecoder *tc, struct timecode_def *def,
                    double speed, unsigned int sample_rate);
void timecoder_clear(struct timecoder *tc);

int timecoder_monitor_init(struct timecoder *tc, int size);
void timecoder_monitor_clear(struct timecoder *tc);

void timecoder_cycle_definition(struct timecoder *tc);
void timecoder_submit(struct timecoder *tc, const signed short *pcm, size_t npcm);
signed int timecoder_get_position(struct timecoder *tc, double *when);

/*
 * The timecode definition currently in use by this decoder
 */

static inline struct timecode_def* timecoder_get_definition(struct timecoder *tc)
{
    return tc->def;
}

/*
 * Return the pitch relative to reference playback speed
 */

static inline double timecoder_get_pitch(struct timecoder *tc)
{
    return pitch_current(&tc->pitch) / tc->speed;
}

/*
 * The last 'safe' timecode value on the record. Beyond this value, we
 * probably want to ignore the timecode values, as we will hit the
 * label of the record.
 */

static inline unsigned int timecoder_get_safe(struct timecoder *tc)
{
    return tc->def->safe;
}

/*
 * The resolution of the timecode. This is the number of bits per
 * second at reference playback speed
 */

static inline double timecoder_get_resolution(struct timecoder *tc)
{
    return tc->def->resolution * tc->speed;
}

/*
 * The number of revolutions per second of the timecode vinyl,
 * used only for visual display
 */

static inline double timecoder_revs_per_sec(struct timecoder *tc)
{
    return (33.0 + 1.0 / 3) * tc->speed / 60;
}

#endif
