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

#ifndef TIMECODER_H
#define TIMECODER_H

#include <stdbool.h>

#include "filters.h"
#include "lut.h"
#include "lut_mk2.h"
#include "pitch.h"
#include "delayline.h"

#define TIMECODER_CHANNELS 2

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef unsigned int bits_t;

struct timecode_def {
    const char *name, *desc;
    int bits, /* number of bits in string */
        resolution, /* wave cycles per second */
        flags;
    bits_t seed, /* LFSR value at timecode zero */
        taps; /* central LFSR taps, excluding end taps */
    mk2bits_t seed_mk2, /* MK2 version */
        taps_mk2; /* MK2 version */
    unsigned int length, /* in cycles */
        safe; /* last 'safe' timecode number (for auto disconnect) */
    bool lookup; /* true if lut has been generated */
    struct lut lut;
    struct lut_mk2 lut_mk2; /* MK2 version */
};

struct timecoder_channel_mk2 {
    int rms, rms_deriv; /* RMS values for the signal and its derivative */
    signed int deriv, deriv_scaled; /* Derivative and its scaled version */

    struct delayline delayline; /* needed for the Traktor MK2 demodulation */
    struct ema_filter ema_filter;
    struct differentiator differentiator;
    struct root_mean_square rms_filter, rms_deriv_filter;
};

struct timecoder_channel {
    bool positive, /* wave is in positive part of cycle */
	swapped; /* wave recently swapped polarity */
    signed int zero;
    unsigned int crossing_ticker; /* samples since we last crossed zero */

    struct timecoder_channel_mk2 mk2;
};

struct mk2_subcode {
    mk2bits_t bitstream;
    mk2bits_t timecode;
    mk2bits_t bit;

    unsigned int valid_counter;
    signed int avg_reading;
    signed int avg_slope;
    bool recent_bit_flip;

    struct delayline readings;
    struct ema_filter ema_reading;
    struct ema_filter ema_slope;
};

struct timecoder {
    struct timecode_def *def;
    double speed;

    /* Precomputed values */

    double dt, zero_alpha;
    int sample_rate;
    signed int threshold;

    /* Pitch information */

    bool forwards;
    struct timecoder_channel primary, secondary;
    struct pitch pitch;

    /* Numerical timecode */

    signed int ref_level;
    bits_t bitstream, /* actual bits from the record */
        timecode; /* corrected timecode */
    mk2bits_t mk2_bitstream, /* Traktor MK2 version */
        mk2_timecode; /* Traktor MK2 version */
    unsigned int valid_counter, /* number of successful error checks */
        timecode_ticker; /* samples since valid timecode was read */
    double dB; /* Decibels to detect phono level */

    /* Feedback */

    unsigned char *mon; /* x-y array */
    int mon_size, mon_counter;

    struct mk2_subcode upper_bitstream, lower_bitstream;
    double gain_compensation; /* Scaling factor for the derivative */
};

struct timecode_def* timecoder_find_definition(const char *name, const char *lut_dir_path);
void timecoder_free_lookup(void);

void timecoder_init(struct timecoder *tc, struct timecode_def *def,
                    double speed, unsigned int sample_rate, bool phono);
void timecoder_clear(struct timecoder *tc);

int timecoder_monitor_init(struct timecoder *tc, int size);
void timecoder_monitor_clear(struct timecoder *tc);

void timecoder_cycle_definition(struct timecoder *tc);
void timecoder_submit(struct timecoder *tc, signed short *pcm, size_t npcm);
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

#ifdef __cplusplus
};
#endif // __cplusplus

#endif
