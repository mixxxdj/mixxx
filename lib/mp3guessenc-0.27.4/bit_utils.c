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


#include "mp3g_io_config.h"
#include <string.h>
#include "bit_utils.h"

part23buf_t p23b[NP23B];
int p23b_in=0,p23b_out=0;
int p23b_pos=0;  /* offset of the current bit inside the current output block p23b[p23b_out] */


/* extract a bit stream (up to 32 bit long) out of a byte array,
   starting at a 'ptr'+'begin_ptr' bit */
/* returns the required bit stream, which is long 'length' bits */
/* the 'begin_ptr' (may be NULL) is moved to the next unread bit */
unsigned int extract_bits(unsigned char *ptr, unsigned int *begin_ptr, char length)
{
    unsigned int available_bits;
    unsigned int result = 0;
    unsigned int pointer, byte_pointer;

    /* check */
    if (ptr != NULL && length > 0)
    {
        /* do some setup */
        if (begin_ptr != NULL)
        {
            pointer = *begin_ptr;
            (*begin_ptr) += (unsigned int)length;
        }
        else
        {
            pointer = result;
        }
        byte_pointer = pointer / 8;                  /* first byte to access for bit extraction */

        available_bits = 8 - (pointer%8);            /* evaluate available bits into the current byte */
        if (available_bits < 8)
        {
            result = (unsigned int)ptr[byte_pointer] & ((1<<available_bits)-1);
            byte_pointer++;
            /* maybe we just finished ? */
            if ((unsigned int)length < available_bits)
            {
                result >>= (available_bits-length);
                length = 0;
            }
            else
            {
                length -= available_bits;
            }
        }

        /* extract whole bytes (if any) */
        while (length >= 8)
        {
            result <<= 8;
            result |= (unsigned int)ptr[byte_pointer];
            byte_pointer++;
            length -= 8;
        }

        /* copy remaining bits */
        if (length)
        {
            result <<= length;
            result |= (unsigned int)ptr[byte_pointer] >> (8-length);
        }
    }

    return result;
}


/*
 * `seek_p23b' searches for the requested bit inside the p23b structure
 * (given `pos' as an absolute position) and seeks to that bit when it's found,
 * returning 1 as OK (the indexes `p23b_out' and `p23b_pos' are updated as well).
 * When not found, the function returns zero.
 */
char seek_p23b(bitoffs_t pos)
{
    int idx,rev_step=p23b_in-1;
    char ret=0;
    for(idx=0; idx<NP23B; idx++)
    {
        if (rev_step==-1) rev_step = NP23B-1;
        if (p23b[rev_step].pos <= pos && pos < p23b[rev_step].pos+(bitoffs_t)p23b[rev_step].len)
        {
            /* `p23b_out' will point the entry containing the requested bit */
            p23b_out = rev_step;
            /* `p23b_pos' will be the relative position of the requested bit inside the selected entry */
            p23b_pos = (int)(pos-p23b[p23b_out].pos);
            ret=1;
            break;
        }
        rev_step--;
    }
    return ret;
}

/*
 * This function seeks forward `len' bits into the p23b structure
 * and sets `p23b_out' and `p23b_pos' indexes accordingly.
 * If unable to seek (not enough data) it returns the spare bits,
 * else zero.
 */
int skip_p23b(int len)
{
    int temp;

    while(len)
    {
        if (p23b[p23b_out].len-p23b_pos <= len)
        {
            /* the bits I have in the current block are not enough for the requested seek,
               so I will have to jump to the next block.
               First of all, I have to check whether it exists or not */
            temp = (p23b_out+1) % NP23B;
            if (temp == p23b_in)
            {
                /* no more blocks, can't move to the next one! */
                len -= p23b[p23b_out].len-p23b_pos-1;
                p23b_pos = p23b[p23b_out].len-1;
                break;
            }
            else
            {
                len -= p23b[p23b_out].len-p23b_pos;
                p23b_out = temp;
                p23b_pos = 0;
            }
        }
        else
        {
            p23b_pos += len;
            len = 0;
        }
    }
    return len;
}

/*
 * This function seeks back `len' bits into the p23b structure
 * and sets `p23b_out' and `p23b_pos' indexes accordingly.
 * If unable to seek (not enough data) it returns the spare bits,
 * else zero.
 */
int skipback_p23b(int len)
{
    int temp;

    while(len)
    {
        if (p23b_pos+1 <= len)
        {
            temp = p23b_out-1;
            if (temp == -1) temp = NP23B-1;

            if (temp == p23b_in)
            {
                /* there is no previous block! Cannot move... */
                len -= p23b_pos;
                p23b_pos = 0;
                break;
            }
            else
            {
                /* moving to the previous block */
                len -= p23b_pos+1;
                p23b_out = temp;
                p23b_pos = p23b[p23b_out].len-1;
            }
        }
        else
        {
            p23b_pos -= len;
            len = 0;
        }
    }
    return len;
}

bitoffs_t tell_p23b(void)
{
    return (bitoffs_t)p23b_pos + p23b[p23b_out].pos;
}


void load_into_p23b(unsigned char *buff, bitoffs_t bit_pos, int block_len)
{
    if (block_len > 0)
    {
        /* store the offset of the beginning of part2 data (bits) with respect to the entire file stream */
        p23b[p23b_in].pos = bit_pos;

        if (block_len>LARGEST_FRAME)
        {
            printf("Warning: requested loading for a block too large - fixing.\n");
            block_len=LARGEST_FRAME;
        }

        memcpy(p23b[p23b_in].buf, buff, block_len);
        /* set length of data block */
        p23b[p23b_in].len=block_len*8;

        if (p23b_in == p23b_out)
            p23b_pos = 0;
        p23b_in = (p23b_in+1) % NP23B;
    }
}


// read byte-aligned 8 bits
/*
 * NOTE: the old `readOneByte_p23b' returns unreliable bytes!
 * It stays assured that the caller has already checked for byte availability,
 * so it goes straight on extracting and returning a byte.
 * The wrong is, the byte availability comes from an approximation:
 * if the start bit and the end bit differ by more than 8 bit, then there is
 * (at least) an available byte BUT
 * it happens the start bit and the end bit lie on different part2+part3 buffer entries
 * and when it comes to extract the very last byte of the sequence, it may happen
 * to extract a byte belonging to the beginning of the subsequent part2+part3 block
 * because it is stored into the next part2+part3 buffer entry. The last-but-one byte
 * is right, though.
 * The reason is they do have a lot of bytes in between (even more than one hundred)
 * but the actual length of remaining ancillary data is less than 8 bit.
 */

/*
 * `p23b_cpy' gets inspired by the well known `memcpy' string functions.
 * Since I often need to copy several byte sequences, it appears faster to switch
 * to memcpy and obtain a whole sequence copied in a row.
 * Due to the new enhanced precision, using `p23b_cpy' mp3guessenc may now extract
 * packet of ancillary data being 1 byte smaller than before (read above).
 * Incidentally, the missing byte prevents for unwanted characters to appear into the
 * encoder string (so now the reported string will always be `LAME3.96.1' instead of
 * `LAME3.96.1w') for a cleaner encoder identification.
 * Warning: the start bit has to be set BEFORE calling `p23b_cpy' (for example, with
 * a `seek_p23b' call).
 * Return value: how many bytes have been copied
 */
int p23b_cpy(unsigned char *buffer, bitoffs_t endbit, int buffer_size, int *byte_off)
{
    int copied=0;

//    p23b_pos = (p23b_pos+7)&~7; /* round to the next byte */
    *byte_off = p23b_pos % 8;
    p23b_pos = p23b_pos&~7;
    if (p23b_pos >= p23b[p23b_out].len)
    {  /* jump to the next entry */
        p23b_out = (p23b_out + 1) % NP23B;
        p23b_pos = 0;
        if (p23b_out == p23b_in)
            buffer_size = 0;
    }

    while (buffer_size)
    {
        if (endbit <= p23b[p23b_out].pos+(bitoffs_t)p23b_pos)
            /* here I have nothing to copy from */
            break;
        else
            if (((p23b[p23b_out].len-p23b_pos)/8 <= buffer_size) && (p23b[p23b_out].pos+(bitoffs_t)p23b[p23b_out].len <= endbit))
            /* the closer bound is the block end */
            {
                int len = (p23b[p23b_out].len-p23b_pos)/8;
                memcpy(buffer+copied, p23b[p23b_out].buf+p23b_pos/8, len);
                buffer_size -= len;
                copied += len;
                p23b_out = (p23b_out + 1) % NP23B;
                p23b_pos = 0;
                if (p23b_out == p23b_in)
                    break;
            }
            else
                if ( (p23b[p23b_out].pos+(bitoffs_t)p23b_pos < endbit) && (endbit < p23b[p23b_out].pos+(bitoffs_t)p23b[p23b_out].len) && ((int)(endbit-p23b[p23b_out].pos-(bitoffs_t)p23b_pos)/8 <= buffer_size) )
                /* the closer bound is the `endbit' */
                {
                    memcpy(buffer+copied,p23b[p23b_out].buf+p23b_pos/8,(int)((endbit-p23b[p23b_out].pos-p23b_pos)/8));
                    copied += (int)((endbit-p23b[p23b_out].pos-p23b_pos)/8);
                    p23b_pos = (int)(endbit-p23b[p23b_out].pos);
                    break;
                }
                else
                    if ((buffer_size < (p23b[p23b_out].len-p23b_pos)/8) && (p23b[p23b_out].pos+(bitoffs_t)p23b_pos+(bitoffs_t)(buffer_size)*8 < endbit))
                    /* the closer bound is the buffer limit */
                    {
                        memcpy(buffer+copied,p23b[p23b_out].buf+p23b_pos/8,buffer_size);
                        copied += buffer_size;
                        p23b_pos += buffer_size*8;
                        break;
                    }
    }

    return copied;
}
