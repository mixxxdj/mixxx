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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "timecoder.h"

#define ZERO_THRESHOLD 128
#define SIGNAL_THRESHOLD 256

#define ZERO_AVG 1024
#define SIGNAL_AVG 256

#define REF_PEAKS_AVG 48 /* in wave cycles */

/* The number of correct bits which come in before the timecode 
 * is declared valid. Set this too low, and risk the record skipping around 
 * (often to blank areas of track) during scratching */

#define VALID_BITS 24

#define MONITOR_DECAY_EVERY 512 /* in samples */

#define SQ(x) ((x)*(x))


/* Timecode definitions */


#define POLARITY_NEGATIVE 0
#define POLARITY_POSITIVE 1


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
        seed: 0xd8b40,
        tap: {2, 4, 6, 8, 10, 11, 14, 16, 17},
        ntaps: 9,
        length: 910000,
        safe: 900000,
        lookup: NULL
    },
    {
        name: "traktor_a",
        desc: "Traktor Scratch, side A",
        resolution: 2000,
        polarity: POLARITY_POSITIVE,
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
        polarity: POLARITY_POSITIVE,
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


//struct timecode_def_t *def;


/* Linear Feeback Shift Register in the forward direction. New values
 * are generated at the least-significant bit. */

static inline int lfsr(unsigned int code, struct timecoder_t *timecoder)
{
    unsigned int r;
    char s, n;

    r = code & 1;

    for(n = 0; n < timecoder->tc_table->ntaps; n++) {
        s = *(timecoder->tc_table->tap + n);
        r += (code & (1 << s)) >> s;
    }
    
    return r & 0x1;
}


/* Linear Feeback Shift Register in the reverse direction. New values
 * are generated at the most-significant bit. */

static inline int lfsr_rev(unsigned int code, struct timecoder_t *timecoder)
{
    unsigned int r;
    char s, n;

    r = (code & (1 << (timecoder->tc_table->bits - 1))) >> (timecoder->tc_table->bits - 1);

    for(n = 0; n < timecoder->tc_table->ntaps; n++) {
        s = *(timecoder->tc_table->tap + n) - 1;
        r += (code & (1 << s)) >> s;
    }
    
    return r & 0x1;
}


/* Setup globally, for a chosen timecode definition */

int timecoder_build_lookup(char *timecode_name, struct timecoder_t *timecoder) {
    unsigned int n, current;

    struct timecode_def_t *def;
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

    //Copy the lookup table stuff
    if (timecoder->tc_table == NULL) {
        timecoder->tc_table = malloc(sizeof(struct timecode_def_t));
    }
    memcpy(timecoder->tc_table, def, sizeof(struct timecode_def_t));

    fprintf(stderr, "Allocating %d slots (%zuKb) for %d bit timecode (%s)\n",
            2 << timecoder->tc_table->bits, (2 << timecoder->tc_table->bits) * sizeof(unsigned int) / 1024,
            timecoder->tc_table->bits, timecoder->tc_table->desc);

    timecoder->tc_table->lookup = malloc((2 << timecoder->tc_table->bits) * sizeof(unsigned int));
    if(!timecoder->tc_table->lookup) {
        perror("malloc");
        return 0;
    }
   
    for(n = 0; n < ((unsigned int)2 << timecoder->tc_table->bits); n++)
        timecoder->tc_table->lookup[n] = -1;
    
    current = timecoder->tc_table->seed;
    
    for(n = 0; n < timecoder->tc_table->length; n++) {
        if(timecoder->tc_table->lookup[current] != -1) {
            fprintf(stderr, "Timecode has wrapped; finishing here.\n");
            return -1;
        }
        
        timecoder->tc_table->lookup[current] = n;
        current = (current >> 1) + (lfsr(current, timecoder) << (timecoder->tc_table->bits - 1));
        //printf("n=%d\n", n);
    }
    
    return 0;    
}


/* Free the timecoder lookup table when it is no longer needed */

void timecoder_free_lookup(struct timecoder_t* timecoder) {
    if (timecoder->tc_table->lookup)
    {
        free(timecoder->tc_table->lookup);
        timecoder->tc_table->lookup = NULL;
    }
}


static void init_channel(struct timecoder_channel_t *ch)
{
    ch->positive = 0;
    ch->zero = 0;
    ch->crossing_ticker = 0;
}


/* Initialise a timecode decoder */

void timecoder_init(struct timecoder_t *tc)
{
    int c;

    tc->forwards = 1;
    tc->rate = TIMECODER_RATE;

    tc->half_peak = 0;
    tc->wave_peak = 0;
    tc->ref_level = -1;
    tc->signal_level = 0;

    init_channel(&tc->mono);
    for(c = 0; c < TIMECODER_CHANNELS; c++)
        init_channel(&tc->channel[c]);
        
    tc->crossings = 0;
    tc->pitch_ticker = 0;

    tc->bitstream = 0;
    tc->timecode = 0;
    tc->valid_counter = 0;
    tc->timecode_ticker = 0;

    tc->mon = NULL;
    tc->log_fd = -1;
    
    tc->tc_table = NULL;
}


/* Clear a timecode decoder */

void timecoder_clear(struct timecoder_t *tc)
{
    timecoder_monitor_clear(tc);
}


/* The monitor (otherwise known as 'scope' in the interface) is the
 * display of the incoming audio. Initialise one for the given
 * timecoder */

void timecoder_monitor_init(struct timecoder_t *tc, int size, int scale)
{
    tc->mon_size = size;
    tc->mon_scale = scale;
    tc->mon = malloc(SQ(tc->mon_size));
    memset(tc->mon, 0, SQ(tc->mon_size));
    tc->mon_counter = 0;
}


/* Clear the monitor on the given timecoder */

void timecoder_monitor_clear(struct timecoder_t *tc)
{
    if(tc->mon) {
        free(tc->mon);
        tc->mon = NULL;
    }
}


static int detect_zero_crossing(struct timecoder_channel_t *ch,
                                signed short v, int rate)
{
    int swapped;

    ch->crossing_ticker++;

    swapped = 0;
    if(v >= ch->zero + ZERO_THRESHOLD && !ch->positive) {
        swapped = 1;
        ch->positive = 1;
        ch->crossing_ticker = 0;
    } else if(v < ch->zero - ZERO_THRESHOLD && ch->positive) {
        swapped = 1;
        ch->positive = 0;
        ch->crossing_ticker = 0;
    }
    
    ch->zero += (v - ch->zero) * ZERO_AVG / rate;
    
    return swapped;
}


/* Submit and decode a block of PCM audio data to the timecoder */

int timecoder_submit(struct timecoder_t *tc, signed short *pcm, int samples)
{
    int b, l, /* bitstream and timecode bits */
        s, c,
        x, y, p, /* monitor coordinates */
        v,
        offset,
        swapped,
        monitor_centre;
    signed short w; /* pcm sample values */
    unsigned int mask;
    
    b = 0;
    l = 0;
    
    mask = ((1 << tc->tc_table->bits) - 1);
    monitor_centre = tc->mon_size / 2;

    offset = 0;

    for(s = 0; s < samples; s++) {
        
        for(c = 0; c < TIMECODER_CHANNELS; c++)
            detect_zero_crossing(&tc->channel[c], pcm[offset + c], tc->rate);

        /* Read from the mono channel */
        
        v = pcm[offset] + pcm[offset + 1];
        swapped = detect_zero_crossing(&tc->mono, v, tc->rate);

        /* If a sign change in the (zero corrected) audio has
         * happened, log the peak information */
        
        if(swapped) {
            
            /* Work out whether half way through a cycle we are
             * looking for the wave to be positive or negative */
            
            if(tc->mono.positive == (tc->tc_table->polarity ^ tc->forwards)) {
                
                /* Entering the second half of a wave cycle */
                
                tc->half_peak = tc->wave_peak;
                
            } else {
                
                /* Completed a full wave cycle, so time to analyse the
                 * level and work out whether it's a 1 or 0 */
                
                b = tc->wave_peak + tc->half_peak > tc->ref_level;
                
                /* Log binary timecode */
                
                if(tc->log_fd != -1)
                    write(tc->log_fd, b ? "1" : "0", 1);
                
                /* Add it to the bitstream, and work out what we were
                 * expecting (timecode). */
                
                /* tc->bitstream is always in the order it is
                 * physically placed on the vinyl, regardless of the
                 * direction. */
                
                if(tc->forwards) {
                    l = lfsr(tc->timecode, tc);
                    
                    tc->bitstream = (tc->bitstream >> 1)
                        + (b << (tc->tc_table->bits - 1));
                    
                    tc->timecode = (tc->timecode >> 1)
                        + (l << (tc->tc_table->bits - 1));
                    
                } else {
                    l = lfsr_rev(tc->timecode, tc);
                    
                    tc->bitstream = ((tc->bitstream << 1) & mask) + b;
                    tc->timecode = ((tc->timecode << 1) & mask) + l;
                }
                
                if(b == l) {
                    tc->valid_counter++;
                } else {
                    tc->timecode = tc->bitstream;
                    tc->valid_counter = 0;
                }
                
                /* Take note of the last time we read a valid timecode */
                
                tc->timecode_ticker = 0;
                
                /* Adjust the reference level based on the peaks seen
                 * in this cycle */
                
                if(tc->ref_level == -1)
                    tc->ref_level = tc->half_peak + tc->wave_peak;
                else {
                    tc->ref_level = (tc->ref_level * (REF_PEAKS_AVG - 1)
                                     + tc->half_peak + tc->wave_peak)
                        / REF_PEAKS_AVG;
                }
                
            }
            
            /* Calculate the immediate direction from phase difference,
             * based on the last channel to cross zero */

            if(tc->channel[0].crossing_ticker > tc->channel[1].crossing_ticker)
                tc->forwards = 1;
            else
                tc->forwards = 0;

            if(tc->forwards)
                tc->crossings++;
            else
                tc->crossings--;
            
            tc->pitch_ticker += tc->crossing_ticker;
            tc->crossing_ticker = 0;
            tc->wave_peak = 0;
            
        } /* swapped */
        
        tc->crossing_ticker++;
        tc->timecode_ticker++;
        
        /* Find the zero-normalised sample of the peak value from
         * the input */
        
        w = abs(v - tc->mono.zero);
        if(w > tc->wave_peak)
            tc->wave_peak = w;
        
        /* Take a rolling average of zero and signal level */

        tc->signal_level += (w - tc->signal_level) * SIGNAL_AVG / tc->rate;

        /* Update the monitor to add the incoming sample */
        
        if(tc->mon) {
            
            /* Decay the pixels already in the montior */
            
            if(++tc->mon_counter % MONITOR_DECAY_EVERY == 0) {
                for(p = 0; p < SQ(tc->mon_size); p++) {
                    if(tc->mon[p])
                        tc->mon[p] = tc->mon[p] * 7 / 8;
                }
            }
            
            v = pcm[offset]; /* first channel */
            w = pcm[offset + 1]; /* second channel */
            
            x = monitor_centre + (v * tc->mon_size * tc->mon_scale / 32768);
            y = monitor_centre + (w * tc->mon_size * tc->mon_scale / 32768);
            
            /* Set the pixel value to white */
            
            if(x > 0 && x < tc->mon_size && y > 0 && y < tc->mon_size)
                tc->mon[y * tc->mon_size + x] = 0xff;
        }
        
        offset += TIMECODER_CHANNELS;
        
    } /* for each sample */
    
    /* Print debugging information */
    
#if 0
    fprintf(stderr, "%+6d +/%4d -/%4d (%4d,%4d)\t= %d (%d) %c %d"
            "\t[crossings: %d %d]",
            tc->mono.zero,
            tc->half_peak,
            tc->wave_peak,
            tc->ref_level >> 1,
            tc->signal_level,
            b, l, b == l ? ' ' : 'x',
            tc->valid_counter,
            tc->crossings,
            tc->pitch_ticker);

    if(tc->pitch_ticker)
        fprintf(stderr, " = %d", tc->rate * tc->crossings / tc->pitch_ticker);

    fputc('\n', stderr);
#endif

    return 0;
}


/* Return the timecode pitch, based on cycles of the sine wave. This
 * function can only be called by one context, at it resets the state
 * of the counter in the timecoder. */

int timecoder_get_pitch(struct timecoder_t *tc, float *pitch)
{
    /* Let the caller know if there's no data to gather pitch from */

    if(tc->crossings == 0)
        return -1;

    /* Value of tc->crossings may be negative in reverse */
    
    *pitch = tc->rate * (float)tc->crossings / tc->pitch_ticker
        / (tc->tc_table->resolution * 2);

    tc->crossings = 0;
    tc->pitch_ticker = 0;

    return 0;
}


/* Return the known position in the timecode, or -1 if not known. If
 * two few bits have been error-checked, then this also counts as
 * invalid. If 'when' is given, return the time, in input samples since
 * this value was read. */

signed int timecoder_get_position(struct timecoder_t *tc, int *when)
{
    signed int r;

    if(tc->valid_counter > VALID_BITS) {
        r = tc->tc_table->lookup[tc->bitstream];

        if(r >= 0) {
            if(when) 
                *when = tc->timecode_ticker;
            return r;
        }
    }
    
    return -1;
}


/* Return non-zero if there is any timecode signal available */

int timecoder_get_alive(struct timecoder_t *tc)
{
    if(tc->signal_level < SIGNAL_THRESHOLD)
        return 0;
    
    return 1;
}


/* Return the last 'safe' timecode value on the record. Beyond this
 * value, we probably want to ignore the timecode values, as we will
 * hit the label of the record. */

unsigned int timecoder_get_safe(struct timecoder_t *tc)
{
    return tc->tc_table->safe;
}


/* Return the resolution of the timecode. This is the number of bits
 * per second, which corresponds to the frequency of the sine wave */

int timecoder_get_resolution(struct timecoder_t *tc)
{
    return tc->tc_table->resolution;
}
