/*
 *   bit_utils is a support module which provides bit oriented utilities
 *   Copyright (C) 2013-2018 Elio Blanca <eblanca76@users.sourceforge.net>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef BIT_UTILS_H
#define BIT_UTILS_H

/*
 * you can take 8*511 bit
 * from the bit reservoir and at most 8*1440 bit from the current
 * frame (320 kbps, 32 kHz), so 8*1951 bit is the largest possible
 * value for MPEG1 and 2)
 * UPDATE: the biggest supported frame is now 8*5760 bit due to a free
 * format stream (MPEG2.5, 640 kbps, 8 kHz). I won't take into account the further 511 bytes
 * from the bit reservoir because they are expected to be in previous blocks (if any).
 * Further, due to `checkvbrinfotag' seeking for a new sync bit sequence, the buffer has to be
 * large enough to contain a full frame and a new mpeg header (4 bytes).
 */

#define LARGEST_FRAME (5760+4)

// In the worst case, 9 bitstream frames have to be received before decoding can start.
/*
 * UPDATE: the worst case is a MPEG2, 24000 Hz, 8kbps frame which is 24 bytes long (192 bits)
 * Now, I know 32 bits (header), 16 bits (error protection) and 136 bits (side information)
 * are needed for the static part, and 32+16+136=184. This means a single byte (8 bits) is available
 * for storing part2 and part3 of audio data.
 * A complex solution is needed, indeed the whole part of frame decoding needs to be reviewed but
 * as a temporary solution, increasing the number of frames will do the trick
 * ATM, the `More bits in reservoir are needed to decode this frame.' sentence has still to be
 * expected.
 */
#define NP23B 25 //(9+3)

/*
 * This structure will store the bytes for part2 (scalefactors)
 * and part3 (Huffman encoded data) of the main data
 */
typedef struct PART23BUF {
    unsigned char buf[LARGEST_FRAME];
    int           len;
    bitoffs_t     pos;
} part23buf_t;


unsigned int extract_bits(unsigned char *, unsigned int *, char);
char seek_p23b(bitoffs_t);
int skip_p23b(int);
int skipback_p23b(int);
bitoffs_t tell_p23b(void);
void load_into_p23b(unsigned char *, bitoffs_t, int);
int p23b_cpy(unsigned char *, bitoffs_t, int, int *);

#endif
