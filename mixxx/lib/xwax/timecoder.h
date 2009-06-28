/* 
 * Copyright (C) 2008 Mark Hills <mark@pogo.org.uk>
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

#define TIMECODER_CHANNELS 2
#define TIMECODER_RATE 44100 //Default rate - Albert

#define MAX_BITS 32 /* bits in an int */

struct timecode_def_t {
    char *name, *desc;
    int bits, /* number of bits in string */
        resolution, /* wave cycles per second */
        tap[MAX_BITS], ntaps, /* LFSR taps */
        polarity; /* cycle begins POLARITY_POSITIVE or POLARITY_NEGATIVE */
    unsigned int seed, /* LFSR value at timecode zero */
        length, /* in cycles */
        safe; /* last 'safe' timecode number (for auto disconnect) */
    signed int *lookup; /* pointer to built lookup table */
};

struct timecoder_channel_t {
    int positive; /* wave is in positive part of cycle */
    signed int zero;
    int crossing_ticker; /* samples since we last crossed zero */
};


struct timecoder_t {
    int forwards, rate;

    /* Signal levels */

    signed int signal_level, half_peak, wave_peak, ref_level;
    struct timecoder_channel_t mono, channel[TIMECODER_CHANNELS];

    /* Pitch information */

    int crossings, /* number of zero crossings */
        pitch_ticker, /* number of samples from which crossings counted */
        crossing_ticker; /* stored for incrementing pitch_ticker */

    /* Numerical timecode */

    unsigned int bitstream, /* actual bits from the record */
        timecode; /* corrected timecode */
    int valid_counter, /* number of successful error checks */
        timecode_ticker; /* samples since valid timecode was read */

    /* Feedback */

    unsigned char *mon; /* x-y array */
    int mon_size, mon_counter, mon_scale,
        log_fd; /* optional file descriptor to log to, or -1 for none */
        
    struct timecode_def_t *tc_table;
};


/* Building the lookup table is global. Need a good way to share
 * lookup tables soon, so we can use a different timecode on 
 * each timecoder, and switch between them. */

int timecoder_build_lookup(char *timecode_name, struct timecoder_t *timecoder);
void timecoder_free_lookup(struct timecoder_t *timecoder);

void timecoder_init(struct timecoder_t *tc);
void timecoder_clear(struct timecoder_t *tc);

void timecoder_monitor_init(struct timecoder_t *tc, int size, int scale);
void timecoder_monitor_clear(struct timecoder_t *tc);

int timecoder_submit(struct timecoder_t *tc, signed short *aud, int samples);

int timecoder_get_pitch(struct timecoder_t *tc, float *pitch);
signed int timecoder_get_position(struct timecoder_t *tc, int *when);
int timecoder_get_alive(struct timecoder_t *tc);
unsigned int timecoder_get_safe(struct timecoder_t *tc);
int timecoder_get_resolution(struct timecoder_t *tc);

#endif
