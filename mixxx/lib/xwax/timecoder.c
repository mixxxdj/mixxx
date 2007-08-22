/*
 * Copyright (C) 2007 Mark Hills <mark@pogo.org.uk>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "timecoder.h"

#define ZERO_THRESHOLD 128
#define SIGNAL_THRESHOLD 256

#define ZERO_AVG (TIMECODER_RATE / 1024)
#define SIGNAL_AVG (TIMECODER_RATE / 256)

#define MAX_BITS 32 /* bits in an int */

#define REF_PEAKS_AVG 48 /* in wave cycles */

/* The number of correct bits which come in before the timecode 
 * is declared valid. Set this too low, and risk the record skipping around 
 * (often to blank areas of track) during scratching */

#define VALID_BITS 24

#define MONITOR_DECAY_EVERY (TIMECODER_RATE / 128)

#define SQ(x) ((x)*(x))


/* Timecode definitions */


#define POLARITY_NEGATIVE 0
#define POLARITY_POSITIVE 1

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


struct timecode_def_t timecode_def[] = {
    {
        name: "serato_2a",
        desc: "Serato 2nd Ed., side A",
        resolution: 1000,
        polarity: POLARITY_POSITIVE,
        bits: 20,
        seed: 0x59017,
        tap: {2, 5, 6, 7, 8, 13, 14, 16, 17},
        ntaps: 9,
        length: 712000,
        safe: 707000,
        lookup: NULL
    },
    {
        name: "serato_2b",
        desc: "Serato 2nd Ed., side B",
        seed: 0x8f3c6,
        resolution: 1000,
        polarity: POLARITY_POSITIVE,
        bits: 20,
        tap: {3, 4, 6, 7, 12, 13, 14, 15, 18}, /* reverse of side A */
        ntaps: 9,
        length: 922000,
        safe: 917000,
        lookup: NULL
    },
    {
        name: "serato_cd",
        desc: "Serato CD",
        resolution: 1000,
        polarity: POLARITY_POSITIVE,
        bits: 20,
        seed: 0x84c0c,
        tap: {2, 4, 6, 8, 10, 11, 14, 16, 17},
        ntaps: 9,
        length: 940000,
        safe: 930000,
        lookup: NULL
    },
    {
        name: "traktor_a",
        desc: "Traktor Scratch, side A",
        resolution: 2000,
        polarity: POLARITY_NEGATIVE,
        bits: 23,
        seed: 0x134503,
        tap: {6, 12, 18},
        ntaps: 3,
        length: 1500000,
        safe: 1480000,
        lookup: NULL
    },
    {
        name: "traktor_b",
        desc: "Traktor Scratch, side B",
        resolution: 2000,
        polarity: POLARITY_NEGATIVE,
        bits: 23,
        seed: 0x32066c,
        tap: {6, 12, 18},
        ntaps: 3,
        length: 2110000,
        safe: 2090000,
        lookup: NULL
    },
    {
        name: NULL
    }
};


struct timecode_def_t *def;


/* Linear Feeback Shift Register in the forward direction. New values
 * are generated at the least-significant bit. */

static inline int lfsr(unsigned int code)
{
    unsigned int r;
    char s, n;

    r = code & 1;

    for(n = 0; n < def->ntaps; n++) {
        s = *(def->tap + n);
        r += (code & (1 << s)) >> s;
    }
    
    return r & 0x1;
}


/* Linear Feeback Shift Register in the reverse direction. New values
 * are generated at the most-significant bit. */

static inline int lfsr_rev(unsigned int code)
{
    unsigned int r;
    char s, n;

    r = (code & (1 << (def->bits - 1))) >> (def->bits - 1);

    for(n = 0; n < def->ntaps; n++) {
        s = *(def->tap + n) - 1;
        r += (code & (1 << s)) >> s;
    }
    
    return r & 0x1;
}


/* Setup globally, for a chosen timecode definition */

int timecoder_build_lookup(char *timecode_name) {
    unsigned int n, current;

    def = &timecode_def[0];

    while(def->name) {
        if(!strcmp(def->name, timecode_name))
            break;
        def++;
    }

    if(!def->name) {
        fprintf(stderr, "Timecode definition '%s' is not known.\n",
                timecode_name);
        return -1;
    }

    fprintf(stderr, "Allocating %d slots (%zuKb) for %d bit timecode (%s)\n",
            2 << def->bits, (2 << def->bits) * sizeof(unsigned int) / 1024,
            def->bits, def->desc);

    def->lookup = malloc((2 << def->bits) * sizeof(unsigned int));
    if(!def->lookup) {
        perror("malloc");
        return 0;
    }
    
    for(n = 0; n < ((unsigned int)2 << def->bits); n++)
        def->lookup[n] = -1;
    
    current = def->seed;
    
    for(n = 0; n < def->length; n++) {
        if(def->lookup[current] != -1) {
            fprintf(stderr, "Timecode has wrapped; finishing here.\n");
            return -1;
        }
        
        def->lookup[current] = n;
        current = (current >> 1) + (lfsr(current) << (def->bits - 1));
    }
    
    return 0;    
}


/* Free the timecoder lookup table when it is no longer needed */

void timecoder_free_lookup(void) {
    if (def->lookup)
        free(def->lookup);
}


/* Initialise a timecode decoder */

int timecoder_init(struct timecoder_t *tc)
{
    int c;
    struct timecoder_channel_t *st;

    for(c = 0; c < TIMECODER_CHANNELS; c++) {
        st = &tc->state[c];

        st->zero = 0;
        st->wave_peak = 0;
        st->ref_level = -1;
        st->signal_level = 0;

        st->positive = 0;
        st->crossings = 0;
        st->cycle_ticker = 0;
        st->crossings_ticker = 0;
        st->timecode_ticker = 0;

        st->bitstream = 0;
        st->timecode = 0;

        st->valid_counter = 0;
    }

    tc->forwards = 1;
    tc->mon = NULL;
    tc->log_fd = -1;
    
    return 0;
}


/* Clear a timecode decoder */

int timecoder_clear(struct timecoder_t *tc)
{
    timecoder_monitor_clear(tc);
    return 0;
}


/* The monitor (otherwise known as 'scope' in the interface) is the
 * display of the incoming audio. Initialise one for the given
 * timecoder */

int timecoder_monitor_init(struct timecoder_t *tc, int size, int scale)
{
    tc->mon_size = size;
    tc->mon_scale = scale;
    tc->mon = malloc(SQ(tc->mon_size));
    memset(tc->mon, 0, SQ(tc->mon_size));
    tc->mon_counter = 0;

    return 0;
}


/* Clear the monitor on the given timecoder */

int timecoder_monitor_clear(struct timecoder_t *tc)
{
    if(tc->mon) {
        free(tc->mon);
        tc->mon = NULL;
    }
    return 0;
}


/* Submit and decode a block of PCM audio data to the timecoder */

int timecoder_submit(struct timecoder_t *tc, signed short *pcm, int samples)
{
    int b, l, /* bitstream and timecode bits */
        s, c, /* samples, channels */
        x, y, p, /* monitor coordinates */
        t, u, /* phase difference */
        swapped,
        monitor_centre;
    signed short v, w; /* pcm sample values */
    unsigned int mask;
    struct timecoder_channel_t *st, *sto;
    
    b = 0;
    l = 0;

    mask = ((1 << def->bits) - 1);
    monitor_centre = tc->mon_size / 2;

    for(s = 0; s < samples; s++) {

        for(c = 0; c < TIMECODER_CHANNELS; c++) {
            st = &tc->state[c];
            
            v = pcm[s * TIMECODER_CHANNELS + c];
            
            /* Work out if we've crossed through zero, based on a zero
             * being a range rather than simply a value */
            
            swapped = 0;
            
            if(v >= st->zero + ZERO_THRESHOLD && !st->positive) {
                swapped = 1;
                st->positive = 1;
            
            } else if(v < st->zero - ZERO_THRESHOLD && st->positive) {
                swapped = 1;
                st->positive = 0;
            }
            
            /* If a sign change in the (zero corrected) audio has
             * happened, log the peak information */
            
            if(swapped) {

                /* Work out whether half way through a cycle we are
                 * looking for the wave to be positive or negative */

                if(st->positive == (def->polarity ^ tc->forwards)) {

                    /* Entering the second half of a wave cycle */
                    
                    st->half_peak = st->wave_peak;

                } else {
                    
                    /* Completed a full wave cycle, so time to analyse
                     * the level and work out whether it's a 1 or 0 */

                    b = st->wave_peak + st->half_peak > st->ref_level;
                    
                    /* Log _all_ channels, interleaved */

                    if(tc->log_fd != -1 && c == 0)
                        write(tc->log_fd, b ? "1" : "0", 1);
                    
                    /* Add it to the bitstream, and work out what we
                     * were expecting (timecode). */

                    /* st->bitstream is always in the order it is
                     * physically placed on the vinyl, regardless of
                     * the direction. */
                    
                    if(tc->forwards) {
                        l = lfsr(st->timecode);
                        
                        st->bitstream = (st->bitstream >> 1)
                            + (b << (def->bits - 1));

                        st->timecode = (st->timecode >> 1)
                            + (l << (def->bits - 1));

                    } else {
                        l = lfsr_rev(st->timecode);

                        st->bitstream = ((st->bitstream << 1) & mask) + b;
                        st->timecode = ((st->timecode << 1) & mask) + l;
                    }

                    if(b == l) {
                        st->valid_counter++;
                    } else {
                        st->timecode = st->bitstream;
                        st->valid_counter = 0;
                    }

                    /* Take note of the last time we read a valid
                     * timecode */
                    
                    st->timecode_ticker = 0;

                    /* Adjust the reference level based on the peaks
                     * seen in this cycle */
                    
                    if(st->ref_level == -1)
                        st->ref_level = st->half_peak + st->wave_peak;
                    else {
                        st->ref_level = (st->ref_level * (REF_PEAKS_AVG - 1)
                                         + st->half_peak + st->wave_peak)
                            / REF_PEAKS_AVG;
                    }
    
                }

                /* Calculate the immediate direction based on phase
                 * difference of the two channels */
                
                if(c == 0) {
                
#if 0
    st = &tc->state[0];

    fprintf(stderr, "%+6d +/%4d -/%4d (%d)\t= %d (%d) %c %d"
            "\t[crossings: %d %d]\n",
            st->zero,
            st->half_peak,
            st->wave_peak,
            st->ref_level >> 1,
            b, l, b == l ? ' ' : 'x',
            st->valid_counter,
            st->crossings,
            st->crossings_ticker);
#endif
                
                    sto = &tc->state[!c];
                    
                    t = st->cycle_ticker;
                    u = sto->cycle_ticker;
                    
                    if(t > u) {
                        if(st->positive == sto->positive) {
                            st->crossings++;
                            tc->forwards = 1;
                        }
                        
                        if(st->positive != sto->positive) {
                            st->crossings--;
                            tc->forwards = 0;
                        }
                    }
                }                    
                
                /* Reset crossing couner */
                
                st->crossings_ticker += st->cycle_ticker;
                st->cycle_ticker = 0;
                st->wave_peak = 0;

            } /* swapped */

            st->cycle_ticker++;
            st->timecode_ticker++;

            /* Find the zero-normalised sample of the peak value from
             * the input */

            w = abs(v - st->zero);
            if(w > st->wave_peak)
                st->wave_peak = w;

            /* Take a rolling average of zero and signal level */

            st->zero = (st->zero * (ZERO_AVG - 1) + v) / ZERO_AVG;

            st->signal_level = (st->signal_level * (SIGNAL_AVG - 1) + w)
                / SIGNAL_AVG;

        } /* for each channel */

        /* Update the monitor to add the incoming sample */
        
        if(tc->mon) {
            
            /* Decay the pixels already in the montior */
            
            if(++tc->mon_counter % MONITOR_DECAY_EVERY == 0) {
                for(p = 0; p < SQ(tc->mon_size); p++)
                    tc->mon[p] *= 0.9;
            }

            v = pcm[s * TIMECODER_CHANNELS]; /* first channel */
            w = pcm[s * TIMECODER_CHANNELS + 1]; /* second channel */
            
            x = monitor_centre + (v * tc->mon_size * tc->mon_scale / 32768);
            y = monitor_centre + (w * tc->mon_size * tc->mon_scale / 32768);

            /* Set the pixel value to white */

            if(x > 0 && x < tc->mon_size && y > 0 && y < tc->mon_size)
                tc->mon[y * tc->mon_size + x] = 0xff;
        }

    } /* for each sample */

    /* Print debugging information */



    return 0;
}


/* Return the timecode pitch, based on cycles of the sine wave. This
 * function can only be called by one context, at it resets the state
 * of the counter in the timecoder. */

int timecoder_get_pitch(struct timecoder_t *tc, float *pitch)
{
    struct timecoder_channel_t *st;

    st = &tc->state[0];
    
    /* Let the caller know if there's no data to gather pitch from */

    if(st->crossings_ticker == 0)
        return -1;

    /* Value of st->crossings may be negative */
    
    *pitch = TIMECODER_RATE * (float)st->crossings / st->crossings_ticker
        / (def->resolution * 2);
    
    st->crossings = 0;
    st->crossings_ticker = 0;

    return 0;
}


/* Return the known position in the timecode, or -1 if not known. If
 * two few bits have been error-checked, then this also counts as
 * invalid. If 'when' is given, return the time, in input samples when
 * this value was read. */

signed int timecoder_get_position(struct timecoder_t *tc, int *when)
{
    struct timecoder_channel_t *st;
    signed int r;

    st = &tc->state[0];

    if(st->valid_counter > VALID_BITS) {
        r = def->lookup[tc->state[0].bitstream];

        if(r >= 0) {
            if(when) 
                *when = st->timecode_ticker;
            
            return r;
        }
    }
    
    return -1;
}


/* Return non-zero if there is any timecode signal available */

int timecoder_get_alive(struct timecoder_t *tc)
{
    struct timecoder_channel_t *st;

    st = &tc->state[0];

    if(st->signal_level < SIGNAL_THRESHOLD)
        return 0;
    
    return 1;
}


/* Return the last 'safe' timecode value on the record. Beyond this
 * value, we probably want to ignore the timecode values, as we will
 * hit the label of the record. */

unsigned int timecoder_get_safe(struct timecoder_t *tc)
{
    return def->safe;
}


/* Return the resolution of the timecode. This is the number of bits
 * per second, which corresponds to the frequency of the sine wave */

int timecoder_get_resolution(struct timecoder_t *tc)
{
    return def->resolution;
}
