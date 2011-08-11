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

#ifndef TIMECODER_H
#define TIMECODER_H

#ifndef _MSC_VER
#include <stdbool.h>
#endif

/* #include "device.h" */
#include "lut.h"
#include "pitch.h"

#define TIMECODER_CHANNELS 2


typedef unsigned int bits_t;


struct timecode_def_t {
    char *name, *desc;
    int bits, /* number of bits in string */
        resolution, /* wave cycles per second */
        flags;
    bits_t seed, /* LFSR value at timecode zero */
        taps; /* central LFSR taps, excluding end taps */
    unsigned int length, /* in cycles */
        safe; /* last 'safe' timecode number (for auto disconnect) */
    bool lookup; /* true if lut has been generated */
    struct lut_t lut;
};


struct timecoder_channel_t {
    int positive, /* wave is in positive part of cycle */
	swapped; /* wave recently swapped polarity */
    signed int zero;
    unsigned int crossing_ticker; /* samples since we last crossed zero */
};


struct timecoder_t {
    struct timecode_def_t *def;
    double speed;

    /* Precomputed values */

    float dt, zero_alpha;

    /* Pitch information */

    int forwards;
    struct timecoder_channel_t primary, secondary;
    struct pitch_t pitch;

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


void timecoder_free_lookup(void);

int timecoder_init(struct timecoder_t *tc, const char *def_name, double speed,
                   unsigned int sample_rate);
void timecoder_clear(struct timecoder_t *tc);

int timecoder_monitor_init(struct timecoder_t *tc, int size);
void timecoder_monitor_clear(struct timecoder_t *tc);

void timecoder_submit(struct timecoder_t *tc, const signed short *pcm, size_t npcm);

signed int timecoder_get_position(struct timecoder_t *tc, float *when);


/* Return the pitch relative to reference playback speed */

static inline float timecoder_get_pitch(struct timecoder_t *tc)
{
    return pitch_current(&tc->pitch) / tc->speed;
}


/* The last 'safe' timecode value on the record. Beyond this value, we
 * probably want to ignore the timecode values, as we will hit the
 * label of the record. */

static inline unsigned int timecoder_get_safe(struct timecoder_t *tc)
{
    return tc->def->safe * 1000 / (tc->def->resolution * tc->speed);
}


/* The resolution of the timecode. This is the number of bits per
 * second at reference playback speed */

static inline double timecoder_get_resolution(struct timecoder_t *tc)
{
    return tc->def->resolution * tc->speed;
}


/* The number of revolutions per second of the timecode vinyl,
 * used only for visual display */

static inline double timecoder_revs_per_sec(struct timecoder_t *tc)
{
    return (33.0 + 1.0 / 3) * tc->speed / 60;
}

#endif
