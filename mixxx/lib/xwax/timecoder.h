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

#ifndef TIMECODER_H
#define TIMECODER_H

//Hacked by Albert:
#define TIMECODER_CHANNELS 2
#define TIMECODER_RATE 44100 //this is now timecoder_t.samplerate


struct timecoder_channel_t {
    signed short zero, signal_level, half_peak, wave_peak, ref_level;

    int positive, /* wave is in positive part of cycle */
        crossings; /* number of zero crossings */

    unsigned int bitstream, /* actual bits from the record */
        timecode, /* corrected timecode */
        valid_counter, /* number of successful error checks */
        crossings_ticker, /* number of samples from which crossings counted */
        cycle_ticker, /* samples since wave last crossed zero */
        timecode_ticker; /* samples since valid timecode was read */
};

struct timecoder_t {
    struct timecoder_channel_t state[TIMECODER_CHANNELS];
    int forwards;

    unsigned long samplerate; /* sampling rate of the audio stream */ 
    unsigned char *mon; /* visual monitor of waveform */
    int mon_size, mon_counter, mon_scale,
        log_fd; /* optional file descriptor to log to, or -1 for none */
    unsigned long zero_avg, /* ??? */
        signal_avg, /* ??? */
        monitor_decay_every; /* ??? */ 
};


/* Building the lookup table is global. Need a good way to share
 * lookup tables soon, so we can use a different timecode on 
 * each timecoder, and switch between them. */

int timecoder_build_lookup(char *timecode_name);
void timecoder_free_lookup(void);

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
