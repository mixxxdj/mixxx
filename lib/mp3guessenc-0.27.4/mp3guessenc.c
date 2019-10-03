/*
 *   mp3guessenc is an mp3 encoder-guesser
 *   Copyright (C) 2002-2010 Naoki Shibata
 *   Copyright (C) 2011-2018 Elio Blanca <eblanca76@users.sourceforge.net>
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

/* mp3guessenc version 0.2 by Naoki Shibata */
/* This program is still under development. */
/* Modifed by Evan Dekker 2019-09-26 */


#define VER_PKG_NAME    "mp3guessenc"
#define VER_MAJ         0
#define VER_MIN         27
#define VER_PATCH_LEV   4
#define CODENAME        "It's happened before it'll happen again"

#define REL_TYPE        0  /* release type 0:stable, 1:alpha, 2:beta, 3:preview, 4:releasecandidate */
#define REL_SUBNUM      1

#define ENABLE_SKIP_OPTION


/*
 * release type is provided via RELEASE_STABLE/RELEASE_ALPHA/RELEASE_BETA/RELEASE_PREVIEW/RELEASE_RC macros
 *
 * So please, do not use REL_TYPE for release check!
 * Again, check which is true among RELEASE_STABLE/RELEASE_ALPHA/RELEASE_BETA/RELEASE_PREVIEW/RELEASE_RC
 */

#ifdef REL_TYPE
#define RELEASE_STABLE  (REL_TYPE==0)
#define RELEASE_ALPHA   (REL_TYPE==1)
#define RELEASE_BETA    (REL_TYPE==2)
#define RELEASE_PREVIEW (REL_TYPE==3)
#define RELEASE_RC      (REL_TYPE==4)
#else
#define RELEASE_STABLE  0
#define RELEASE_ALPHA   1
#define RELEASE_BETA    0
#define RELEASE_PREVIEW 0
#define RELEASE_RC      0
#endif

#if (RELEASE_STABLE)
#undef REL_SUBNUM
#endif


#include "mp3g_io_config.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#if defined(__WINDOWS__)
#include <io.h>
#else
#include <unistd.h>
#endif
#include <sys/stat.h>

#include "mp3guessenc.h"
#include "bit_utils.h"
#ifdef CODENAME
#include "decode.h"
#include "scrambled.h"
#define CMD_CODENAME_LENGTH 80 /* max length of a string provided via command line */
#endif


#define BLOCKCOUNT_LONG                 0
#define BLOCKCOUNT_SHORT                1
#define BLOCKCOUNT_MIXED                2
#define BLOCKCOUNT_SWITCH               3
#define BLOCKCOUNT_TYPES                4

#define MODE_PLAIN_STEREO               0
#define MODE_JOINT_STEREO               1
#define MODE_DUAL_CHANNEL               2
#define MODE_MONO                       3

#define MPEG_CODE_MPEG2_5               0
#define MPEG_CODE_RESERVED              1
#define MPEG_CODE_MPEG2                 2
#define MPEG_CODE_MPEG1                 3

#define SAMPLERATE_CODE_RESERVED        3

#define EMPHASIS_CODE_RESERVED          2

#define PRO_SIGN_BYTE1               0xC0
#define PRO_SIGN_BYTE2               0x08
#define SURR_SIGN_BYTE1              0xCF
#define SURR_SIGN_BYTE2              0x30

#define OFL_IN_MP3PRO_BYTE_1         0xE0
#define OFL_IN_MP3PRO_BYTE_2         0xBA
#define OFL_IN_MP3SURROUND_BYTE_1    0xB4
#define OFL_IN_MP3SURROUND_BYTE_2    0x08
#define OFL_IN_PLAINMP3_BYTE_1       OFL_IN_MP3SURROUND_BYTE_1
#define OFL_IN_PLAINMP3_BYTE_2       0x04

#define GLOBAL_GAIN_CHANNEL_LEFT        0
#define GLOBAL_GAIN_CHANNEL_RIGHT       1
#define GLOBAL_GAIN_CHANNEL_MONO        GLOBAL_GAIN_CHANNEL_LEFT

#define SIZEOF_OFF64_T                  8

#define ENCODER_GUESS_GOGO                         18
#define ENCODER_GUESS_LAME_OLD                     17
#define ENCODER_GUESS_LAME                         16
#define ENCODER_GUESS_MP3PRO                       15
#define ENCODER_GUESS_MP3PRO_OFL                   14
#define ENCODER_GUESS_MP3SURR                      13
#define ENCODER_GUESS_MP3SURR_OFL                  12
#define ENCODER_GUESS_MP3S_ENC                     11
#define ENCODER_GUESS_FHG_FASTENC                  10
#define ENCODER_GUESS_FHG_MAYBE_FASTENC             9
#define ENCODER_GUESS_FHG_ACM                       8
#define ENCODER_GUESS_FHG_MAYBE_L3ENC               7
#define ENCODER_GUESS_XING_VERY_OLD                 6
#define ENCODER_GUESS_XING_NEW                      5
#define ENCODER_GUESS_XING_OLD                      4
#define ENCODER_GUESS_BLADE                         3
#define ENCODER_GUESS_HELIX                         2
#define ENCODER_GUESS_UNKNOWN                       1

char *encoder_table[] =
{
    "unused entry",
    "dist10 encoder or other encoder",
    "Helix",
    "BladeEnc",
    "Xing (old)",
    "Xing (new)",
    "Xing (very old)",
    "FhG (l3enc, fastenc or mp3enc)",
    "FhG (ACM or producer pro)",
    "FhG fastenc, mp3sEncoder or mp3 plugin",
    "FhG fastenc",
    "FhG mp3sEncoder with OFL",
    "Fraunhofer IIS mp3Surround 5.1 encoder with OFL",
    "Fraunhofer IIS mp3Surround 5.1 encoder",
    "Fraunhofer IIS mp3PRO encoder with OFL",
    "Fraunhofer IIS mp3PRO encoder",
    "Lame",
    "Lame (old) or m3e",
    "Gogo"
};


typedef enum
{
    HEAD_NO_TAG,
    HEAD_ID3V2_TAG,
    HEAD_APE_TAG,
    HEAD_WAVERIFF_UNCOMPLETE_TAG,
    HEAD_WAVERIFF_DATACHUNK_TAG
} head_metadata_tag_t;


int samples_tab[2][3] = {{ 384,1152,1152},                  /* MPEG1:     lI, lII, lIII */
                         { 384,1152, 576}};                 /* MPEG2/2.5: lI, lII, lIII */

int samplerate_tab [3][4] = {{11025,99999,22050,44100},     /* MPEG2.5, resv, MPEG2, MPEG1 */
                             {12000,99999,24000,48000},     /* MPEG2.5, resv, MPEG2, MPEG1 */
                             { 8000,99999,16000,32000}};    /* MPEG2.5, resv, MPEG2, MPEG1 */

int bitrate_tab[2][3][16] = {
  { {128, 32, 64, 96,128,160,192,224,256,288,320,352,384,416,448,},      /* MPEG1 lI   */
    {128, 32, 48, 56, 64, 80, 96,112,128,160,192,224,256,320,384,},      /* MPEG1 lII  */
    {128, 32, 40, 48, 56, 64, 80, 96,112,128,160,192,224,256,320,} },    /* MPEG1 lIII */

  { {128, 32, 48, 56, 64, 80, 96,112,128,144,160,176,192,224,256,},      /* MPEG2/2.5 lI   */
    {128,  8, 16, 24, 32, 40, 48, 56, 64, 80, 96,112,128,144,160,},      /* MPEG2/2.5 lII  */
    {128,  8, 16, 24, 32, 40, 48, 56, 64, 80, 96,112,128,144,160,} }     /* MPEG2/2.5 lIII */
};

unsigned char subband_limit[5]={27,30,8,12,30};             /* sub band limits for mpeg layerII */
unsigned char subband_quanttable[5][30]=
{
    /* subband quantization table for mpeg layerII */
    {4,4,4,4,4,4,4,4,4,4,4,3,3,3,3,3,3,3,3,3,3,3,3,2,2,2,2,0,0,0},
    {4,4,4,4,4,4,4,4,4,4,4,3,3,3,3,3,3,3,3,3,3,3,3,2,2,2,2,2,2,2},
    {4,4,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {4,4,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {4,4,4,4,3,3,3,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2}
};

mmtag_t musicmatch_tag;
unsigned char mp3g_storage[LARGEST_BUFFER];


char malformed_part1(unsigned char *, currentFrame *);

/* Mpeg header utilities */


char mp3guessenc_head_check(unsigned int head, char relaxed)
/*
 * Integrity checks for an mpeg header (4 bytes)
 * Return values:
 *  0: invalid header
 *  1: valid header
 * when `relaxed' is nonzero, then the check on bitrate index is skipped
 * but only for layerIII !
 */
{
    char result=0;
    unsigned char ly=(head&HEADER_FIELD_LAYER)  >>HEADER_FIELD_LAYER_SHIFT;            /* layer index */
    unsigned char bi=(head&HEADER_FIELD_BITRATE)>>HEADER_FIELD_BITRATE_SHIFT;          /* bitrate index */
    unsigned char mp=(head&HEADER_FIELD_MPEG_ID)>>HEADER_FIELD_MPEG_ID_SHIFT;          /* mpeg version */
    unsigned char md;                                                                  /* audio mode */

    if ((head&HEADER_FIELD_SYNC) == HEADER_FIELD_SYNC)          /* check for sync bits */
    {
        if (mp != MPEG_CODE_RESERVED)                           /* check for reserved value in mpeg version field */
        {
            if (ly!=LAYER_CODE_RESERVED)                        /* check for reserved value in layer field */
            {
                if (((head&HEADER_FIELD_SAMPRATE)>>HEADER_FIELD_SAMPRATE_SHIFT) != SAMPLERATE_CODE_RESERVED )  /* check for reserved value in sample rate field */
                {
                    if ((head&HEADER_FIELD_EMPHASIS) != EMPHASIS_CODE_RESERVED)       /* check for reserved value in emphasis field */
                    {
                        if ((relaxed && ly==LAYER_CODE_L_III)
                            ||
                           bi!=BITRATE_INDEX_RESERVED)          /* check for invalid bitrate index */
                        {
                            result = 1;
                        }
                    }
                }
            }
        }
    }

    /* the header is valid and the mpeg frame is supported! */

    if (result && ly==LAYER_CODE_L__II && mp==MPEG_CODE_MPEG1)
    {
        /* if mpeg 1 layerII, then check for not allowed bitrate-mode combinations */
        if (bi != BITRATE_INDEX_FREEFORMAT)         /* free format bitstreams allow for any bitrate-mode combinations */
        {
            md=(head&HEADER_FIELD_CHANNELS)>>HEADER_FIELD_CHANNELS_SHIFT;
            if (
                (bi<4  && md!=MODE_MONO)
                ||
                (bi==5 && md!=MODE_MONO)
                ||
                (bi>10 && md==MODE_MONO)
               )
            {
                result = 0;
            }
            else
            {
                /* all went ok, this mpeg1layerII header is valid */
            }
        }
    }

    return result;
}

int get_framesize(unsigned int head)
/*
 * "Just give me an audio mpeg header,
 * I will give you in return its frame size"
 * Works with mpeg1, mpeg2, mpeg2.5, all layers
 * BUT not with freeformat frames
 */
{
    unsigned char lsf,layer,bitrate_index,srate_index,mpeg,padding,samplesfactor=1;
    int size;

    lsf           = 1-((head&HEADER_FIELD_LSF)  >> HEADER_FIELD_LSF_SHIFT);
    layer         = 3-((head&HEADER_FIELD_LAYER)>> HEADER_FIELD_LAYER_SHIFT);   /* this goes from 0 (layerI) up to 2 (layerIII) */
    bitrate_index = (head&HEADER_FIELD_BITRATE) >> HEADER_FIELD_BITRATE_SHIFT;
    srate_index   = (head&HEADER_FIELD_SAMPRATE)>> HEADER_FIELD_SAMPRATE_SHIFT;
    mpeg          = (head&HEADER_FIELD_MPEG_ID) >> HEADER_FIELD_MPEG_ID_SHIFT;
    padding       = (head&HEADER_FIELD_PADDING) >> HEADER_FIELD_PADDING_SHIFT;

/*
 * This `samplesfactor' is used to compensate for rounding errors when managing integers only. The
 * size formula is right just without this (aside from the pad*4) but it keeps not returning
 * multiple-of-4 values. So this way it is fixed for working in any case with layers II and III.
 * For layer I, I just multiply by 4 before returning the size.
 */
    if (!layer) samplesfactor=4;

    size = samples_tab[lsf][layer]/samplesfactor*bitrate_tab[lsf][layer][bitrate_index]*125/samplerate_tab[srate_index][mpeg]+(int)padding;

    if (!layer)
        size *= 4;

    return size;
}

float get_bitrate (unsigned int header, int framesize)
{
    unsigned char lsf,layer,srate_index,mpeg,padding/*,samplesfactor=1*/;
    float bitrate;

    lsf           = 1-((header&HEADER_FIELD_LSF)  >> HEADER_FIELD_LSF_SHIFT);
    layer         = 3-((header&HEADER_FIELD_LAYER)>> HEADER_FIELD_LAYER_SHIFT);   /* this goes from 0 (layerI) up to 2 (layerIII) */
    srate_index   = (header&HEADER_FIELD_SAMPRATE)>> HEADER_FIELD_SAMPRATE_SHIFT;
    mpeg          = (header&HEADER_FIELD_MPEG_ID) >> HEADER_FIELD_MPEG_ID_SHIFT;
    padding       = (header&HEADER_FIELD_PADDING) >> HEADER_FIELD_PADDING_SHIFT;

    if (!layer)
        padding *= 4;
//        samplesfactor=4;

//    bitrate = (float)((framesize-padding)*samplerate_tab[srate_index][mpeg]*samplesfactor)/(125.0*(float)samples_tab[lsf][layer]);
    bitrate = (float)((framesize-padding)*samplerate_tab[srate_index][mpeg])/(125.0*(float)samples_tab[lsf][layer]);

    return bitrate;
}

char freeformat(unsigned int head)
{
    return (((head&HEADER_FIELD_BITRATE)>>HEADER_FIELD_BITRATE_SHIFT)==BITRATE_INDEX_FREEFORMAT);
}

char layerI(unsigned int head)
{
    return (((head&HEADER_FIELD_LAYER)>>HEADER_FIELD_LAYER_SHIFT)==LAYER_CODE_L___I);
}

char layerII(unsigned int head)
{
    return (((head&HEADER_FIELD_LAYER)>>HEADER_FIELD_LAYER_SHIFT)==LAYER_CODE_L__II);
}

char layerIII(unsigned int head)
{
    return (((head&HEADER_FIELD_LAYER)>>HEADER_FIELD_LAYER_SHIFT)==LAYER_CODE_L_III);
}

/* unit for begin_ptr and length is bit */
/* begin_ptr should have values in 1..7 - fix: it works with any values */
/* begin_ptr may be NULL, when not NULL it is increased of 'length' amount */
unsigned short mp3guessenc_crc_update(unsigned short old_crc, unsigned char *data, unsigned int *begin_ptr, unsigned int length)
{
#define CRC16_POLYNOMIAL 0x8005

    unsigned char idx, data_len;
    unsigned int new_data, crc1=(unsigned int)old_crc, byte_length,
                 begin_offs=0, next_8multiple, jdx;

    if (begin_ptr != NULL && length > 0)
    {
        next_8multiple = (*begin_ptr+7) & ~7;
        if ((next_8multiple - *begin_ptr) > 0)
        {
            new_data = (unsigned int)(data[*begin_ptr/8]);
            new_data = new_data<<((*begin_ptr%8)+8);
            if (length < (next_8multiple - *begin_ptr))
                data_len = (unsigned char)length;
            else
                data_len = (unsigned char)(next_8multiple - *begin_ptr);
            /* 'data_len' is into [1..7] at most */

            for (idx=0; idx<data_len; idx++)
            {
                new_data <<= 1;
                crc1 <<= 1;

                if (((crc1 ^ new_data) & 0x10000))
                    crc1 ^= CRC16_POLYNOMIAL;
            }

            *begin_ptr += data_len;
            length -= data_len;
        }

        begin_offs = next_8multiple/8;
        *begin_ptr += length;
    }

    byte_length = length / 8;

    for (jdx=0; jdx<byte_length; jdx++)
    {
        new_data = (unsigned int)(data[begin_offs+jdx]);
        new_data = new_data<<8;
        for (idx=0; idx<8; idx++)
        {
            new_data <<= 1;
            crc1 <<= 1;

            if (((crc1 ^ new_data) & 0x10000))
                crc1 ^= CRC16_POLYNOMIAL;
        }
    }

    if (length % 8)
    {
        new_data = (unsigned int)(data[begin_offs+byte_length]);
        new_data = new_data<<8;
        for (idx=0; idx<length%8; idx++)
        {
            new_data <<= 1;
            crc1 <<= 1;

            if (((crc1 ^ new_data) & 0x10000))
                crc1 ^= CRC16_POLYNOMIAL;
        }
    }

    return (unsigned short)(crc1&0xFFFF);
}

char verify_crc(unsigned int header, unsigned char *chk_buffer, int p1len, unsigned short target)
{
    unsigned char dummy;
    unsigned short curr_crc = 0xFFFF;

    /* the crc16 is first calculated on the last two bytes of the header */
    dummy = (unsigned char)((header>>8)&0xFF);
    curr_crc = mp3guessenc_crc_update(curr_crc, &dummy, NULL, 8);
    dummy = (unsigned char)(header&0xFF);
    curr_crc = mp3guessenc_crc_update(curr_crc, &dummy, NULL, 8);

    curr_crc = mp3guessenc_crc_update(curr_crc, chk_buffer, NULL, p1len-
                          8*(sizeof(unsigned int)+sizeof(unsigned short)));

    return (curr_crc==target);
}

unsigned char reflect_byte(unsigned char by)
{
    unsigned char reflected_nibble[16]={
        0x00, 0x08, 0x04, 0x0c, 0x02, 0x0a, 0x06, 0x0e,
        0x01, 0x09, 0x05, 0x0d, 0x03, 0x0b, 0x07, 0x0f
    };

    return (unsigned char)(
        /* reflect higher nibble */
        reflected_nibble[(by & 0xf0)>>4] +
        /* reflect lower nibble */
        (reflected_nibble[by & 0x0f]<<4) );
}


/*
 * in 'crc_reflected_update' length is in bytes
 */
unsigned short crc_reflected_update(unsigned short old_crc, unsigned char *data, unsigned int length)
{
    unsigned char value1;
    unsigned int idx;

    for (idx=0; idx<length; idx++)
    {
        value1 = reflect_byte(data[idx]);
        old_crc = mp3guessenc_crc_update(old_crc, &value1, NULL, 8);
    }

    /* will I need some reflection here for non zero sums? NOT HERE THE CALLER WILL DO */
    return old_crc;
}

void scan_multichannel(streamInfo *si, unsigned char *buf, unsigned int start_pointer, unsigned int ancill_len, currentFrame *current_frame)
{
/*
 * this routine is heavily based upon the sample mpeg multichannel code into
 * standards.iso.org/ittf/PubliclyAvailableStandards/c039486_ISO_IEC_13818-5_2005_Reference_Software.zip
 * (audio samples into
 * standards.iso.org/ittf/PubliclyAvailableStandards/ISO_IEC_13818-4_2004_Conformance_Testing/Audio/)
 * and its derivative works, such as mctoolame (sourceforge.net/projects/mctoolame).
 */
#define MC_EXTENDED_BITSTREAM_POS                0
#define MC_CENTER_POS                            1
#define MC_SURROUND_POS                          3
#define MC_LFE_CHANNEL_POS                       5
#define MC_NO_MULTI_LINGUAL_POS                  9
#define MC_MULTI_LINGUAL_FS                     12
#define MC_MULTI_LINGUAL_LAYER                  13
#define MC_HEADER_LENGTH                        16
#define MC_CRC_LENGTH                           16
#define MC_TC_SBGR_SELECT_POS                   32
#define MC_DYN_CROSS_ON_POS                     33
#define MC_PREDICTION_ON_POS                    34

    struct fr_params
    {
        unsigned char alloc_bits;
        unsigned char dyn_cross_bits;
        unsigned char mc_channels;
        unsigned char pred_mode;
    } fr_param_table[4][2] =
    {
        /* 0 */ { { 0, 0, 0, 5}, { 2, 1, 1, 2} },
        /* 1 */ { { 2, 1, 1, 4}, { 3, 3, 2, 1} },
        /* 2 */ { { 2, 3, 2, 3}, { 3, 4, 3, 0} },
        /* 3 */ { { 0, 0, 2, 5}, { 2, 1, 3, 2} }
    };

    unsigned char pred_coeff_table[6][16]={
        { 6, 4, 4, 4, 2, 2, 2, 0, 2, 2, 2, 0, 0, 0, 0, 0},
        { 4, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        { 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        { 4, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        { 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    };

    unsigned char sbgr_table[30]={  0,  1,  2,  3,  4,  5,  6,  7,  8,  8,  9,  9, 10, 10, 10,
                                   10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11};

    unsigned char idx, jdx, ext_bs_present, lfe_present, center, surround, tc_sbgr_select,
                  dyn_cross_on, mc_prediction_on, predict, dyn_cross_mode[12], sblimit, layer2,
                  *alloc_table, no_of_multi_lingual_channels, multi_lingual_fs, multi_lingual_layer,
                  ch_start, channels, sbgr, dyn_cross_LR=0, dyn_second_stereo[12], tc_alloc[12];
    unsigned short crc_old, crc_new;
    unsigned int pointer;
    struct fr_params *fr_pr;

    pointer = 13;
    /* extract layer from the frame header */
    layer2 = (extract_bits(buf, &pointer, 2) == 2);

    /* select allocation table and subband limit */
    pointer = 20;
    /* extract sample rate from the frame header */
    if (extract_bits(buf, &pointer, 2) == 1)
    {
        /* 48 kHz */
        alloc_table = subband_quanttable[0];
        sblimit = subband_limit[0];
    }
    else
    {
        /* 32 kHz or 44.1 kHz */
        alloc_table = subband_quanttable[1];
        sblimit = subband_limit[1];
    }

    /* get the channel configuration for main bitstream */
    pointer = 24;
    ch_start = ((extract_bits(buf, &pointer, 2) == MODE_MONO)? 1 : 2);

    /* collect mc header info */

    pointer = start_pointer + MC_EXTENDED_BITSTREAM_POS;
    ext_bs_present = (unsigned char)extract_bits(buf, &pointer, 1);

    /* check for minimal requirements: mpeg1 and data availability */
    pointer = 11;
    if (extract_bits(buf, &pointer, 2) != MPEG_CODE_MPEG1 ||
        (ext_bs_present && ancill_len < 48) ||
        (!ext_bs_present && ancill_len < 800))
    {
        si->mc.mc_stream_verified = 0;
        return;
    }

//if (ext_bs_present) printf("n_ad_bytes=%d\n",extract_bits(buf, &pointer, 8));
    /* begin crc computation */
    pointer = start_pointer;
    crc_new = mp3guessenc_crc_update(0xffff, buf, &pointer, MC_HEADER_LENGTH+8*ext_bs_present);

    pointer = start_pointer + MC_CENTER_POS + 8*ext_bs_present;
    center = (unsigned char)extract_bits(buf, &pointer, 2);

    pointer = start_pointer + MC_SURROUND_POS + 8*ext_bs_present;
    surround = (unsigned char)extract_bits(buf, &pointer, 2);

    fr_pr = &fr_param_table[surround][(center&1)];

    /* total number of channels */
    channels = ch_start + fr_pr->mc_channels;

    pointer = start_pointer + MC_LFE_CHANNEL_POS + 8*ext_bs_present;
    lfe_present = (unsigned char)extract_bits(buf, &pointer, 1);

    pointer = start_pointer + MC_NO_MULTI_LINGUAL_POS + 8*ext_bs_present;
    no_of_multi_lingual_channels = (unsigned char)extract_bits(buf, &pointer, 3);

    pointer = start_pointer + MC_MULTI_LINGUAL_FS + 8*ext_bs_present;
    multi_lingual_fs = (unsigned char)extract_bits(buf, &pointer, 1);

    pointer = start_pointer + MC_MULTI_LINGUAL_LAYER + 8*ext_bs_present;
    multi_lingual_layer = (unsigned char)extract_bits(buf, &pointer, 1);

    pointer = start_pointer + MC_HEADER_LENGTH + 8*ext_bs_present;
    crc_old = (unsigned short)extract_bits(buf, &pointer, MC_CRC_LENGTH);

    pointer = start_pointer + MC_TC_SBGR_SELECT_POS + 8*ext_bs_present;
    tc_sbgr_select = (unsigned char)extract_bits(buf, &pointer, 1);

    pointer = start_pointer + MC_DYN_CROSS_ON_POS + 8*ext_bs_present;
    dyn_cross_on = (unsigned char)extract_bits(buf, &pointer, 1);

    pointer = start_pointer + MC_PREDICTION_ON_POS + 8*ext_bs_present;
    mc_prediction_on = (unsigned char)extract_bits(buf, &pointer, 1);

    if (tc_sbgr_select)
    {
        tc_alloc[0] = (unsigned char)extract_bits(buf, &pointer, fr_pr->alloc_bits);
        for (idx=1; idx<12; idx++)
            tc_alloc[idx] = tc_alloc[0];
    }
    else
    {
        for (idx=0; idx<12; idx++)
            tc_alloc[idx] = (unsigned char)extract_bits(buf, &pointer, fr_pr->alloc_bits);
    }

    memset(dyn_cross_mode,    0, 12*sizeof(unsigned char));
    memset(dyn_second_stereo, 0, 12*sizeof(unsigned char));
    if (dyn_cross_on)
    {
        dyn_cross_LR = (unsigned char)extract_bits(buf, &pointer, 1);
        for (idx=0; idx<12; idx++)
        {
            dyn_cross_mode[idx] = (unsigned char)extract_bits(buf, &pointer, fr_pr->dyn_cross_bits);
            if (surround == 3)
                dyn_second_stereo[idx] = (unsigned char)extract_bits(buf, &pointer, 1);
        }
    }

    if (mc_prediction_on)
        for (idx=0; idx<8; idx++)
        {
            predict = (unsigned char)extract_bits(buf, &pointer, 1);
            if (predict)
            {
                pointer += (2 * pred_coeff_table[fr_pr->pred_mode][dyn_cross_mode[idx]]);
            }
        }

    if (lfe_present)
        pointer += 4;


    /* scan stream */
    if (!dyn_cross_on)
    {
        for (idx=0; idx<sblimit; idx++)
            for (jdx=ch_start; jdx<channels; jdx++)
                if (center != 3 || idx < 12 || jdx != 2)
                    current_frame->allocation[jdx][idx] = (unsigned char)extract_bits(buf, &pointer, alloc_table[idx]);
                else
                    current_frame->allocation[jdx][idx] = 0;
    }
    else
    {
        for (idx=0; idx<sblimit; idx++)
        {
            sbgr = sbgr_table[idx];

            if (dyn_cross_mode[sbgr] == 0)
            {
                for (jdx=ch_start; jdx<channels; jdx++)
                    if (center == 3 && idx >= 12 && jdx == 2)
                        current_frame->allocation[jdx][idx] = 0;
                    else
                        if (surround == 3 && dyn_second_stereo[sbgr] == 1)
                        {
                            if (center != 0 && jdx == 4)
                                current_frame->allocation[jdx][idx] =
                                    current_frame->allocation[3][idx];
                            else
                            {
                                if (center == 0 && jdx == 3)
                                    current_frame->allocation[jdx][idx] =
                                        current_frame->allocation[2][idx];
                                else
                                    current_frame->allocation[jdx][idx] =
                                        (unsigned char)extract_bits(buf, &pointer, alloc_table[idx]);
                            }
                        }
                        else
                            current_frame->allocation[jdx][idx] =
                                (unsigned char)extract_bits(buf, &pointer, alloc_table[idx]);
            }
            else
            {
                switch (fr_pr->dyn_cross_bits)
                {
                    case 1:
                    {
                        /* channel modes 3/0 and 2/1 */
                        if (center == 3 && idx >= 12)
                            /* 3/0 + phantom center */
                            current_frame->allocation[2][idx] = 0;
                        else
                            if (tc_alloc[sbgr] == 1)
                                current_frame->allocation[2][idx] =
                                    current_frame->allocation[0][idx];
                            else
                                if (tc_alloc[sbgr] == 2 || dyn_cross_LR)
                                    current_frame->allocation[2][idx] =
                                        current_frame->allocation[1][idx];
                                else
                                    current_frame->allocation[2][idx] =
                                        current_frame->allocation[0][idx];

                        if (surround == 3)
                        {
                            /* 3/0 and 2/0 */
                            current_frame->allocation[3][idx] =
                                (unsigned char)extract_bits(buf, &pointer, alloc_table[idx]);
                            if (dyn_second_stereo[sbgr] == 1)
                                current_frame->allocation[4][idx] =
                                    current_frame->allocation[3][idx];
                            else
                                current_frame->allocation[4][idx] =
                                    (unsigned char)extract_bits(buf, &pointer, alloc_table[idx]);
                        }
                    }
                    break;

                    case 3:
                    {
                        /* channel modes 3/1 and 2/2 */
                        if (center == 3 && idx >= 12)
                            current_frame->allocation[2][idx] = 0;
                        else
                            if (dyn_cross_mode[sbgr] == 1 || dyn_cross_mode[sbgr] == 4)
                                current_frame->allocation[2][idx] =
                                    (unsigned char)extract_bits(buf, &pointer, alloc_table[idx]);
                            else
                                if ((surround == 2 || tc_alloc[sbgr] == 1 ||
                                    tc_alloc[sbgr] == 5 || tc_alloc[sbgr] != 2)
                                    && !dyn_cross_LR)
                                    current_frame->allocation[2][idx] =
                                        current_frame->allocation[0][idx];
                                else
                                    current_frame->allocation[2][idx] =
                                        current_frame->allocation[1][idx];

                        if (dyn_cross_mode[sbgr] == 2)
                            current_frame->allocation[3][idx] =
                                (unsigned char)extract_bits(buf, &pointer, alloc_table[idx]);
                        else
                            if (dyn_cross_mode[sbgr] == 4)
                                current_frame->allocation[3][idx] =
                                    current_frame->allocation[2][idx];
                            else
                                if ((surround == 2 || tc_alloc[sbgr] == 4 ||
                                    tc_alloc[sbgr] == 5 || tc_alloc[sbgr] < 3)
                                    && dyn_cross_LR)
                                    current_frame->allocation[3][idx] =
                                        current_frame->allocation[1][idx];
                                else
                                    current_frame->allocation[3][idx] =
                                        current_frame->allocation[0][idx];
                    }
                    break;

                    case 4:
                    {
                        /* channel mode 3/2 */
                        if (center == 3 && idx >= 12)
                            current_frame->allocation[2][idx] = 0;
                        else
                            switch (dyn_cross_mode[sbgr])
                            {
                                case 1: case 2: case 4: case 8:
                                case 9: case 10: case 11: case 12:
                                case 14:
                                    current_frame->allocation[2][idx] =
                                        (unsigned char)extract_bits(buf, &pointer, alloc_table[idx]);
                                break;
                                case 3: case 5: case 6: case 7:
                                case 13:
                                    if (tc_alloc[sbgr] == 1 || tc_alloc[sbgr] == 7)
                                        current_frame->allocation[2][idx] =
                                            current_frame->allocation[0][idx];
                                    else
                                        if (tc_alloc[sbgr] == 2 || tc_alloc[sbgr] == 6 || dyn_cross_LR)
                                            current_frame->allocation[2][idx] =
                                                current_frame->allocation[1][idx];
                                        else
                                            current_frame->allocation[2][idx] =
                                                current_frame->allocation[0][idx];
                                break;
                                default:
                                break;
                            }

                        switch (dyn_cross_mode[sbgr])
                        {
                            case 1: case 3: case 5: case 8:
                            case 10: case 13:
                                current_frame->allocation[3][idx] =
                                    (unsigned char)extract_bits(buf, &pointer, alloc_table[idx]);
                                break;
                            case 2: case 4: case 6: case 7:
                            case 12:
                                current_frame->allocation[3][idx] =
                                    current_frame->allocation[0][idx];
                                break;
                            case 9: case 11: case 14:
                                current_frame->allocation[3][idx] =
                                    current_frame->allocation[2][idx];
                            break;
                            default:
                            break;
                        }

                        switch (dyn_cross_mode[sbgr])
                        {
                            case 2: case 3: case 6: case 9:
                                current_frame->allocation[4][idx] =
                                    (unsigned char)extract_bits(buf, &pointer, alloc_table[idx]);
                            break;
                            case 1: case 4: case 5: case 7:
                            case 11:
                                current_frame->allocation[4][idx] =
                                    current_frame->allocation[1][idx];
                            break;
                            case 10: case 12: case 14:
                                current_frame->allocation[4][idx] =
                                    current_frame->allocation[2][idx];
                            break;
                            case 8: case 13:
                                current_frame->allocation[4][idx] =
                                    current_frame->allocation[3][idx];
                            break;
                            default:
                            break;
                        }
                    }
                    break;

                    default:
                        /* fr_pr->dyn_cross_bits == 0 */
                    break;
                }
            }
        }
    }


    for (idx=0; idx<sblimit; idx++)
        for (jdx=ch_start; jdx<channels; jdx++)
            if (current_frame->allocation[jdx][idx])
                /* two more bits for scalefactor selection info */
                pointer += 2;


    /* end gathering data - now check the results */
    if (pointer-start_pointer <= ancill_len)
    {

        start_pointer += MC_TC_SBGR_SELECT_POS + 8*ext_bs_present;
        crc_new = mp3guessenc_crc_update(crc_new, buf, &start_pointer, pointer-start_pointer);


        if (crc_new == crc_old)
        {
            //printf("Success!\n");
            si->mc.mc_stream_verified  |= 1;
            si->mc.extension           = ext_bs_present;
            si->mc.lfe                 = lfe_present;
            si->mc.mc_channels         = fr_pr->mc_channels;
            si->mc.multi_lingual       = no_of_multi_lingual_channels;
            si->mc.multi_lingual_fs    = multi_lingual_fs;
            si->mc.multi_lingual_layer = multi_lingual_layer;
            si->mc.configuration_value = (surround<<4)+center;
            si->mc_verified_frames++;
        }
        else
        {
//            printf("Failure @ frame %d  (read %d bit) :-(\n", si->totFrameNum,pointer-start_cancellare);
            /* may this be a valid incomplete frame? */
            if (!layer2)
            {
                /* take into account we do NOT collect allocation data for layerIII
                   and for layerI they seem not to be consistent with those from
                   additional channels */
                if (si->mc.extension           == ext_bs_present &&
                    si->mc.lfe                 == lfe_present &&
                    si->mc.mc_channels         == fr_pr->mc_channels &&
                    si->mc.multi_lingual       == no_of_multi_lingual_channels &&
                    si->mc.multi_lingual_fs    == multi_lingual_fs &&
                    si->mc.multi_lingual_layer == multi_lingual_layer &&
                    si->mc.configuration_value == (surround<<4)+center)
                {
                    si->mc.mc_stream_verified |= 0x80;
                    si->mc_coherent_frames++;
                }
                else
                {
                    if (si->mc.extension           == 0 &&
                        si->mc.lfe                 == 0 &&
                        si->mc.mc_channels         == 0 &&
                        si->mc.multi_lingual       == 0 &&
                        si->mc.multi_lingual_fs    == 0 &&
                        si->mc.multi_lingual_layer == 0 &&
                        si->mc.configuration_value == 0)
                    {
                        si->mc.extension           = ext_bs_present;
                        si->mc.lfe                 = lfe_present;
                        si->mc.mc_channels         = fr_pr->mc_channels;
                        si->mc.multi_lingual       = no_of_multi_lingual_channels;
                        si->mc.multi_lingual_fs    = multi_lingual_fs;
                        si->mc.multi_lingual_layer = multi_lingual_layer;
                        si->mc.configuration_value = (surround<<4)+center;
                        si->mc.mc_stream_verified |= 0x80;
                        si->mc_coherent_frames = 1;
                    }
                }
            }
        }
    }
    else
    {
//        printf("Incoherent data read, aborting (read %d bit, avail %d bit).\n",pointer-start_pointer,ancill_len);
        if (ext_bs_present)
        {
            /* maybe ancillary room was not enough */
            if (si->mc.extension           == ext_bs_present &&
                si->mc.lfe                 == lfe_present &&
                si->mc.mc_channels         == fr_pr->mc_channels &&
                si->mc.multi_lingual       == no_of_multi_lingual_channels &&
                si->mc.multi_lingual_fs    == multi_lingual_fs &&
                si->mc.multi_lingual_layer == multi_lingual_layer &&
                si->mc.configuration_value == (surround<<4)+center)
            {
                si->mc.mc_stream_verified |= 0x80;
                si->mc_coherent_frames++;
            }
            else
            {
                if (si->mc.extension           == 0 &&
                    si->mc.lfe                 == 0 &&
                    si->mc.mc_channels         == 0 &&
                    si->mc.multi_lingual       == 0 &&
                    si->mc.multi_lingual_fs    == 0 &&
                    si->mc.multi_lingual_layer == 0 &&
                    si->mc.configuration_value == 0)
                {
                    si->mc.extension           = ext_bs_present;
                    si->mc.lfe                 = lfe_present;
                    si->mc.mc_channels         = fr_pr->mc_channels;
                    si->mc.multi_lingual       = no_of_multi_lingual_channels;
                    si->mc.multi_lingual_fs    = multi_lingual_fs;
                    si->mc.multi_lingual_layer = multi_lingual_layer;
                    si->mc.configuration_value = (surround<<4)+center;
                    si->mc.mc_stream_verified |= 0x80;
                    si->mc_coherent_frames = 1;
                }
            }
        }
    }

}


off_t resync_mpeg(FILE *fp, off_t pos, unsigned char **storage, streamInfo *si, char relaxed)
/*
 * Scan the mpeg stream for sync bits
 * input params:
 *           fp  the current file pointer
 *          pos  a starting offset
 *      storage  the resulting frame pointer (optional)
 *               it is guaranteed the buffer will be large enough
 *               to contain a whole mpeg frame
 *           si  stream info pointer (optional) used to know
 *               the size of the frame just found
 *      relaxed  when 1, makes the scan take as valid also layerIII
 *               headers having 1111b (reserved) in the bitrate field
 *               (old versions of lame used to set this in vbr
 *               tags of free format streams)
 * return:
 *               the offset of the new frame found
 *               -1 in case of invalid pos / no valid header found
 *      storage  when valid, it holds the new buffer pointer
 *
 */
{
    unsigned int head;
    int search_length, idx, min_buffer_size;
    off_t result=-1;
    static off_t actual_pos=-1;                /* filepos of the first byte into mp3g_storage */
    static int actual_storage=0;               /* how many valid bytes we store into mp3g_storage */

    if (pos>=0 && fp!=NULL)
    {

        /* first step: ensure under 'pos' I can read a valid byte stream */
        if (pos<actual_pos || pos>=actual_pos+actual_storage-(int)sizeof(unsigned int))
        {
            fseeko(fp, pos, SEEK_SET);
            actual_pos = pos;
            actual_storage = fread(mp3g_storage, 1, LARGEST_BUFFER, fp);
        }


        search_length = SCAN_SIZE;

        idx = pos-actual_pos;
        head = (unsigned int)mp3g_storage[idx+0]<<16 |
               (unsigned int)mp3g_storage[idx+1]<< 8 |
               (unsigned int)mp3g_storage[idx+2];

        /* second step: cycle until I'll find a valid mpeg header */
        while (search_length>0 && actual_storage>3)
        {
            head = (head<<8) | (unsigned int)mp3g_storage[idx+3];

            if (mp3guessenc_head_check(head, relaxed))
            {
                result = actual_pos+(off_t)idx;
                break;
            }

            search_length--;
            idx++;

            if (idx>actual_storage-(int)sizeof(unsigned int))
            {
                /* seek at the beginning of the header being analyzed */
                fseeko(fp, actual_pos+(off_t)idx-sizeof(unsigned int), SEEK_SET);
                actual_pos += (off_t)idx-sizeof(unsigned int);
                idx = 0;

                if ((actual_storage = fread(mp3g_storage, 1, LARGEST_BUFFER, fp)) == 0)
                    /* eof reached */
                    break;

                head = (unsigned int)mp3g_storage[idx+0]<<16 |
                       (unsigned int)mp3g_storage[idx+1]<< 8 |
                       (unsigned int)mp3g_storage[idx+2];
            }
        }

        /* check result */
        if (result != -1)
        {
            /* a valid frame header was found */
            if (si == NULL)
            {
                min_buffer_size = LARGEST_FRAME;
            }
            else
            {
                if (head&HEADER_FIELD_PADDING)
                {
                    min_buffer_size = si->padded[(head&HEADER_FIELD_BITRATE)>>HEADER_FIELD_BITRATE_SHIFT].frameSize;
                }
                else
                {
                    min_buffer_size = si->unpadded[(head&HEADER_FIELD_BITRATE)>>HEADER_FIELD_BITRATE_SHIFT].frameSize;
                }
            }

            /* evaluate remaining bytes */
            if (actual_storage-idx < min_buffer_size && actual_storage == LARGEST_BUFFER)
            {
                /* the buffer is not large enough to hold a whole frame - re-fill! */
                actual_pos += idx;
                idx = 0;
                fseeko(fp, actual_pos, SEEK_SET);
                if ((actual_storage = fread(mp3g_storage, 1, LARGEST_BUFFER, fp)) == 0)
                {
                    /* this happens at the very end of the file (and no trailing metadata) */
                    result = -1;
                }
            }

            /* return the buffer start pointer */
            if (storage != NULL)
            {
                *storage = &mp3g_storage[idx];
            }
        }
    }

    return result;
}

/*
 * this is used to set up `regular' bitrates only
 * in case of free format bitstreams, the setup has already
 * been done into main, thanks to 'checkvbrinfotag' routine
 */
void setup_framesize(unsigned int head, streamInfo *si, detectionInfo *di)
{
    unsigned int i, slot, headbase;

    if (di->lay==1) slot=4;
    else slot=1;

    headbase=head&HEADER_ANY_BUT_BITRATE_AND_PADDING_FIELDS;
    for (i=1;i<15;i++)
    {
        si->unpadded[i].frameSize=get_framesize(headbase|(i<<HEADER_FIELD_BITRATE_SHIFT));
        si->padded[i].frameSize=si->unpadded[i].frameSize+slot;
    }
}

#if 0
void show_me_everything(unsigned int header)
{
    int dummy, lsf, layer, mpeg;

    mpeg = (header&HEADER_FIELD_MPEG_ID)>>HEADER_FIELD_MPEG_ID_SHIFT;
    printf("M%s",((mpeg==0)?"2.5":(mpeg==2)?"2":"1"));
    lsf = (mpeg==0 || mpeg ==2);

    layer = (header&HEADER_FIELD_LAYER)>>HEADER_FIELD_LAYER_SHIFT;
    printf("L%s ",((layer==3)?"I":(layer==2)?"II":"III"));

    dummy = (header&HEADER_FIELD_CRC)>>HEADER_FIELD_CRC_SHIFT;
    if (!dummy) printf("CRC ");

    dummy = (header&HEADER_FIELD_BITRATE)>>HEADER_FIELD_BITRATE_SHIFT;
    if (dummy)
        printf("%dkbps ",bitrate_tab[lsf][3-layer][dummy]);
    else
        printf("ff ");

    dummy = (header&HEADER_FIELD_SAMPRATE)>>HEADER_FIELD_SAMPRATE_SHIFT;
    printf("%dHz ",samplerate_tab[dummy][mpeg]);

    dummy = (header&HEADER_FIELD_PADDING)>>HEADER_FIELD_PADDING_SHIFT;
    if (dummy) printf("pad ");

    dummy = (header&HEADER_FIELD_CHANNELS)>>HEADER_FIELD_CHANNELS_SHIFT;
    printf("%dch ",(dummy==3)?1:2);

    dummy = (header&HEADER_FIELD_COPYRIGHT)>>HEADER_FIELD_COPYRIGHT_SHIFT;
    printf("COPY:%s ",(dummy)?"yes":"no");

    dummy = (header&HEADER_FIELD_ORIGINAL)>>HEADER_FIELD_ORIGINAL_SHIFT;
    printf("ORIG:%s ",(dummy)?"yes":"no");

    dummy = (header&HEADER_FIELD_EMPHASIS)>>HEADER_FIELD_EMPHASIS_SHIFT;
    printf("emph:%s ",((dummy==0)?"none":(dummy==1)?"50/15ms":"CCITT J-17"));

    if (((header&HEADER_FIELD_BITRATE)>>HEADER_FIELD_BITRATE_SHIFT)!=BITRATE_INDEX_FREEFORMAT)
        printf("(%db)",get_framesize(header));
    printf("\n");
}
#endif

/*
 * this will gather information from ancillary data into layerIII
 * streams. Enhanced bitstreams (mp3pro/mp3surrond), OFL (original file
 * length) blocks and lame signatures are detected here.
 */
void scan_ancillary_III(detectionInfo *di, streamInfo *si, int ancill_Len, unsigned char *buf)
{
    if (ancill_Len==0)
    {   /* no ancillary data into the current frame */
        /* this is not expected to happen both in mp3pro and mp3surround
           streams, hence I reset the `enhanced features' bytes */
        di->enhSignature[0] = 0;
        di->enhSignature[1] = 0;
    }
    else
    {
        /* check for an OFL block into the very first frame */
        if (si->totFrameNum==1 && di->lay==3 /* first frame in a layerIII stream */
            &&
            (
            (di->id!=MPEG_CODE_MPEG1 && ancill_Len>20 && (unsigned char)buf[0]==OFL_IN_MP3PRO_BYTE_1 && (unsigned char)buf[1]==OFL_IN_MP3PRO_BYTE_2)
            ||
            (di->id==MPEG_CODE_MPEG1 && ancill_Len> 9 && (unsigned char)buf[0]==OFL_IN_MP3SURROUND_BYTE_1 && ((unsigned char)buf[1]==OFL_IN_MP3SURROUND_BYTE_2 || (unsigned char)buf[1]==OFL_IN_PLAINMP3_BYTE_2))
            )
           )
        {
            di->ofl=1;
            if (di->id!=MPEG_CODE_MPEG1)
            {   /* this is an mp3pro stream with OFL */
                /* at least 20 bytes of ancillary data into the very first frame are used for length information */
                si->ofl_encDelay=extract_bits(buf+14, NULL, 16);
                si->ofl_orig_samples=extract_bits(buf+16, NULL, 32);
                di->enhSignature[0] &= buf[0];
                di->enhSignature[1] &= buf[1];
            }
            else
            {
                /* not an mp3pro stream */
                /* 10 bytes of ancillary data into the very first frame are used for length information */
                si->ofl_encDelay=extract_bits(buf+1, NULL, 16);
                si->ofl_orig_samples=extract_bits(buf+3, NULL, 32);
                if ((unsigned char)buf[1]==OFL_IN_MP3SURROUND_BYTE_2)
                {
                    /* mp3Surround with OFL */
                    si->ofl_encDelay+=576;
                    if (ancill_Len>10) di->enhSignature[0] &= buf[10];
                    if (ancill_Len>11) di->enhSignature[1] &= buf[11];
                }
            }
        }
        else
        {
            if (ancill_Len>0) di->enhSignature[0] = di->enhSignature[0] & buf[0];
            if (ancill_Len>1) di->enhSignature[1] = di->enhSignature[1] & buf[1];
        }
        di->ancillaryData += (off_t)ancill_Len;
        if (ancill_Len>si->ancill_max) si->ancill_max = ancill_Len;
        if (ancill_Len<si->ancill_min) si->ancill_min = ancill_Len;
        extract_enc_string(di->encoder_string,buf,ancill_Len);
    }
}


char read_side_information(unsigned int header, unsigned char *buff, currentFrame *current)
{
    int gg,bigvalues;
    char gr,ngr=((header&HEADER_FIELD_LSF)>>HEADER_FIELD_LSF_SHIFT)+1,ch,
            nch=(((header&HEADER_FIELD_CHANNELS)>>HEADER_FIELD_CHANNELS_SHIFT)==MODE_MONO)?1:2,
                    window_switching_flag,block_type,mixed_block_flag,errors=0;
    unsigned char mpegid=(header&HEADER_FIELD_MPEG_ID)>>HEADER_FIELD_MPEG_ID_SHIFT,
                  mode=(header&HEADER_FIELD_CHANNELS)>>HEADER_FIELD_CHANNELS_SHIFT;
    currentFrame local;
    unsigned int pointer=0;

    memset((void *)&local, 0, sizeof(currentFrame));
    local.min_global_gain[0] = local.min_global_gain[1] = 255;

    if (mpegid==MPEG_CODE_MPEG1)
        local.main_data_begin = extract_bits(buff,&pointer,9);
    else
        local.main_data_begin = extract_bits(buff,&pointer,8);


    /* read private bits */
    if (mpegid==MPEG_CODE_MPEG1)
    {
        if (mode==MODE_MONO)
            extract_bits(buff,&pointer,5);
        else
            extract_bits(buff,&pointer,3);
    }
    else
    {
        if (mode==MODE_MONO)
            extract_bits(buff,&pointer,1);
        else
            extract_bits(buff,&pointer,2);
    }

    /* read scalefactor selection information */
    if (mpegid == MPEG_CODE_MPEG1)
    {
        for(ch=0; ch<nch; ch++)
            local.usesScfsi[(int)ch] = extract_bits(buff,&pointer,4);
    }

    for(gr=0;gr<ngr;gr++)
    {
        for(ch=0;ch<nch;ch++)
        {
            local.part2_3_length += extract_bits(buff,&pointer,12);
            bigvalues = extract_bits(buff,&pointer,9);
            if (bigvalues > 288)
                errors++;
            gg = extract_bits(buff,&pointer,8); /* global_gain */
            if (gg>local.max_global_gain[(int)ch]) local.max_global_gain[(int)ch]=(unsigned char)gg;
            if (gg<local.min_global_gain[(int)ch]) local.min_global_gain[(int)ch]=(unsigned char)gg;

            /* scalefac_compress */
            if (mpegid == MPEG_CODE_MPEG1)
                extract_bits(buff,&pointer,4);
            else
                extract_bits(buff,&pointer,9);

            window_switching_flag = extract_bits(buff,&pointer,1);
            if (window_switching_flag == 1)
            {
                block_type = extract_bits(buff,&pointer,2);
                if (block_type == 0)
                    errors++;
                if (mpegid == MPEG_CODE_MPEG1 && block_type == 2 && local.usesScfsi[(int)ch] != 0)
                    errors++;
                mixed_block_flag = extract_bits(buff,&pointer,1);
                extract_bits(buff,&pointer,5+5+      /* table_select */
                                           3+3+3);   /* subblock_gain */

                if (block_type == 2)
                {
                    if (mixed_block_flag)
                        local.blockCount[BLOCKCOUNT_MIXED]++;  /* this granule contains mixed blocks */
                    else
                        local.blockCount[BLOCKCOUNT_SHORT]++;  /* this granule contains short blocks */
                }
                else
                    local.blockCount[BLOCKCOUNT_SWITCH]++;     /* this granule contains switch blocks */
            }
            else
            {
                extract_bits(buff,&pointer,5+5+5   /* table_select */
                                           +4      /* region0_count */
                                           +3);    /* region1_count */
                local.blockCount[BLOCKCOUNT_LONG]++;      /* this granule contains long blocks */
            }
            /* read preflag */
            if (mpegid == MPEG_CODE_MPEG1)
                extract_bits(buff,&pointer,1);

            local.usesScalefacScale = extract_bits(buff,&pointer,1) | local.usesScalefacScale;
            extract_bits(buff,&pointer,1); /* count1table_select */
        }
    }

    if (current != NULL)
    {
        current->main_data_begin               = local.main_data_begin;
        current->usesScfsi[0]                  = local.usesScfsi[0];
        current->usesScfsi[1]                  = local.usesScfsi[1];
        current->part2_3_length                = local.part2_3_length;
        current->min_global_gain[0]            = local.min_global_gain[0];
        current->min_global_gain[1]            = local.min_global_gain[1];
        current->max_global_gain[0]            = local.max_global_gain[0];
        current->max_global_gain[1]            = local.max_global_gain[1];
        current->blockCount[BLOCKCOUNT_LONG]   = local.blockCount[BLOCKCOUNT_LONG];
        current->blockCount[BLOCKCOUNT_SHORT]  = local.blockCount[BLOCKCOUNT_SHORT];
        current->blockCount[BLOCKCOUNT_MIXED]  = local.blockCount[BLOCKCOUNT_MIXED];
        current->blockCount[BLOCKCOUNT_SWITCH] = local.blockCount[BLOCKCOUNT_SWITCH];
        current->usesScalefacScale             = local.usesScalefacScale;
        current->part1_length                  = pointer + 8 * (sizeof(unsigned int) +
                                                 ((header&HEADER_FIELD_CRC)? 0 : sizeof(unsigned short)));
    }

    return errors;
}


void scan_layer__I(streamInfo *si, detectionInfo *di, currentFrame *current_frame, unsigned char *buf, int verbosity, unsigned char op_flag)
{
/*
 * this routine is heavily based upon the source code of libmad (layer12.c).
 */
    unsigned int pointer;
    int scalefact_len, ch, nch, sb, s, ancillaryLen;

    /* get the channel configuration */
    pointer = 24;
    nch = ((extract_bits(buf, &pointer, 2) == MODE_MONO)? 1 : 2);

    /* scalefactors */
    scalefact_len = 0;
    for (sb=0; sb<32; sb++)
        for (ch=0; ch<nch; ch++)
        {
            if (current_frame->allocation[ch][sb])
                /* each scalefactor is 6 bit long */
                scalefact_len += 6;
        }

    current_frame->part2_3_length = scalefact_len;

    /* samples */
    for (s=0; s<12; s++)
    {
        for (sb=0; sb<current_frame->bound; sb++)
            for (ch=0; ch<nch; ch++)
                current_frame->part2_3_length += current_frame->allocation[ch][sb];
        for (; sb<32; sb++)
            current_frame->part2_3_length += current_frame->allocation[0][sb];
    }

    /* get padding - in layerI a slot is 4 byte long */
    pointer = 22;
    s = extract_bits(buf, &pointer, 1);

    /* check frame integrity */
    if ((current_frame->part1_length+current_frame->part2_3_length) >
        (current_frame->expected_framesize*8))
    {
        /* something wrong here */
        if (verbosity&VERBOSE_FLAG_PROGRESS)
        {
            if (current_frame->uncertain)
                printf("  This frame is broken.\n\n");
            else
            {
                printf("Warning! abnormal length in frame %u (%s padding)\n"
                       "part1=%d part23=%d expected_size=%d\n", si->totFrameNum, ((s)?"with":"no"),
                       current_frame->part1_length, current_frame->part2_3_length, current_frame->expected_framesize);
#if 0
                for (ch=0; ch<2; ch++)
                {
                    for (sb=0; sb<32; sb++)
                        printf(" %2d",current_frame->allocation[ch][sb]);
                    printf("\n");
                }
#endif
            }
        }
    }
    else
    {
        if (current_frame->uncertain)
        {
            if (verbosity&VERBOSE_FLAG_PROGRESS)
                printf("  This frame is valid.\n\n");
        }
        current_frame->uncertain = 0;
        ancillaryLen = (current_frame->expected_framesize*8)
                       - current_frame->part1_length - current_frame->part2_3_length;

        if (op_flag&OPERATIONAL_FLAG_DETECT_MC_IN_ALL_LAYERS)
            scan_multichannel(si, buf, current_frame->part1_length+current_frame->part2_3_length, ancillaryLen, current_frame);

        ancillaryLen /= 8;
        if (s)
        {
            /* this frame has a padding slot */
            ancillaryLen -= 4;
        }

        if (ancillaryLen > 0)
        {
            di->ancillaryData += (off_t)ancillaryLen;
            if (ancillaryLen>si->ancill_max) si->ancill_max = ancillaryLen;
            if (ancillaryLen<si->ancill_min) si->ancill_min = ancillaryLen;
            //printf("ancillary found frame %d, %d bytes\n",si->totFrameNum, ancillaryLen);
        }
    }
}

void scan_layer_II(streamInfo *si, detectionInfo *di, currentFrame *current_frame, unsigned char *buf, int verbosity)
{
/*
 * this routine is heavily based upon the source code of libmad (layer12.c).
 */
    unsigned char halfway_index[5][30]=
    {
        /* offset matrix for samples table */
        {5,5,5,4,4,4,4,4,4,4,4,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0},
        {5,5,5,4,4,4,4,4,4,4,4,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0},
        {3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {3,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3}
    };
    unsigned char samples_table[6][15]=
    {
        /* samples table - derived from offset_table merged with qc_table (quantization classes) */
//      { 0, 1, 16                                                 },
        { 5, 7, 48                                                 },
//      { 0, 1,  2,  3,  4,  5, 16                                 },
        { 5, 7,  9, 10, 12, 15, 48                                 },
//      { 0, 1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14 },
        { 5, 7,  9, 10, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42 },
//      { 0, 1,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15 },
        { 5, 7, 10, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45 },
//      { 0, 1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 16 },
        { 5, 7,  9, 10, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 48 },
//      { 0, 2,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16 }
        { 5, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45, 48 }
    };
    unsigned int pointer;
    int off, scf_len=0, sample_len=0, ancillaryLen;
    unsigned char idx, jdx, sblimit, nch, gr;

    /* get the channel configuration */
    pointer = 24;
    nch = ((extract_bits(buf, &pointer, 2) == MODE_MONO)? 1 : 2);

    sblimit = subband_limit[current_frame->index];

    /* scalefactor section length */
    for (idx=0; idx<sblimit; idx++)
        for (jdx=0; jdx<nch; jdx++)
        {
            if (current_frame->allocation[jdx][idx])
            {
                if (current_frame->scfsi[jdx][idx] == 2)
                {
                    /* single scalefactor */
                    scf_len += 6;
                }
                else
                {
                    if (current_frame->scfsi[jdx][idx] == 0)
                    {
                        /* three scalefactors */
                        scf_len += 18;
                    }
                    else
                    {
                        /* two scalefactors */
                        scf_len += 12;
                    }
                }
            }
        }

    for (gr=0; gr<12; gr++)
    {
        for (idx=0; idx<current_frame->bound; idx++)
            for (jdx=0; jdx<nch; jdx++)
                if (current_frame->allocation[jdx][idx])
                {
                    off = halfway_index[current_frame->index][idx];
                    sample_len += (int)samples_table[off][current_frame->allocation[jdx][idx]-1];
                }
        for (; idx<sblimit; idx++)
            if (current_frame->allocation[0][idx])
            {
                off = halfway_index[current_frame->index][idx];
                sample_len += (int)samples_table[off][current_frame->allocation[0][idx]-1];
            }
    }

    current_frame->part2_3_length = scf_len + sample_len;

    /* get padding - in layerII a slot is 8 bit long */
    pointer = 22;
    idx = extract_bits(buf, &pointer, 1);

    /* check frame integrity */
    if ((current_frame->part1_length+current_frame->part2_3_length) >
        (current_frame->expected_framesize*8))
    {
        /* something wrong here */
        if (verbosity&VERBOSE_FLAG_PROGRESS)
        {
            if (current_frame->uncertain)
                printf("  This frame is broken.\n\n");
            else
            {
                printf("Warning! abnormal length in frame %u (%s padding)\n"
                       "part1=%d part23=%d expected_size=%d\n", si->totFrameNum, ((idx)?"with":"no"),
                       current_frame->part1_length, current_frame->part2_3_length, current_frame->expected_framesize);
#if 0
                for (jdx=0; jdx<2; jdx++)
                {
                    for (sb=0; sb<32; sb++)
                        printf(" %2d",current_frame->allocation[jdx][sb]);
                    printf("\n");
                }
#endif
            }
        }
    }
    else
    {
        if (current_frame->uncertain)
        {
            if (verbosity&VERBOSE_FLAG_PROGRESS)
                printf("  This frame is valid.\n\n");
        }
        current_frame->uncertain = 0;
        ancillaryLen = (current_frame->expected_framesize*8)
                       - current_frame->part1_length - current_frame->part2_3_length;

        scan_multichannel(si, buf, current_frame->part1_length+current_frame->part2_3_length, ancillaryLen, current_frame);

        ancillaryLen /= 8;
        if (idx)
        {
            /* this frame has a padding slot */
            ancillaryLen--;
        }

        if (ancillaryLen > 0)
        {
            di->ancillaryData += (off_t)ancillaryLen;
            if (ancillaryLen>si->ancill_max) si->ancill_max = ancillaryLen;
            if (ancillaryLen<si->ancill_min) si->ancill_min = ancillaryLen;
        }
    }
}

void scan_layerIII(streamInfo *si, detectionInfo *di, bitoffs_t startOfFrame, currentFrame *current_frame,
                   bitoffs_t *endOfLastPart3, unsigned char *buf, unsigned char *local_buff, int local_buff_size,
                   int verbosity, unsigned char op_flag)
{
    char things_went_ok=0;
    bitoffs_t pos_main_data_begin;

    /* we've just read the side information */

    load_into_p23b(buf+current_frame->part1_length/8,
                   startOfFrame+current_frame->part1_length,
                   current_frame->expected_framesize-current_frame->part1_length/8);

    /* first of all, I need to properly set `pos_main_data_begin',
       that is, the offset of the first bit of part2+3 data of the
       current frame --
       in case of insufficient data, `pos_main_data_begin' is set to -1 */
    if (current_frame->main_data_begin == 0)
    {
        pos_main_data_begin = startOfFrame+current_frame->part1_length;
    }
    else
    {   /* MAY THINGS BE EASIER ? Nope.
           Whilst it may appear obvious, I cannot seek to `startOfFrame' position and
           then skip back by `main_data_begin*8' bit, because the `startOfFrame' bit
           belongs to the frame header, and thus it's not been loaded into p23b.
           Instead, the bit `startOfFrame-1' belongs to the previous part2+3 and chances
           are it's already into p23b
         */

        /* if main_data_begin != 0 then it has to be meant as a negative number => it's a real backpointer */
        if (seek_p23b(startOfFrame-1))
        {
            if (skipback_p23b(current_frame->main_data_begin*8-1)) /* if so, then skip back */
            {
                /* not enough data collected */
                if (verbosity&VERBOSE_FLAG_PROGRESS)
                    printf("frame %u : More bits in reservoir are needed to decode this frame.\n",si->totFrameNum);
                pos_main_data_begin = -1;
            }
            else
            {
                /* the seek operation returned ok */
                pos_main_data_begin = tell_p23b();
            }
        }
        else
        {
            /* no data before current part1 data */
            if (verbosity&VERBOSE_FLAG_PROGRESS)
                printf("frame %u : more bits in reservoir are needed to decode this frame.\n",si->totFrameNum);
            pos_main_data_begin = -1;
        }
    }
    /* I put here a simple check in order to prevent useless calls to `scan_ancillary'
     * when I cannot have ancillary data. Of course, having endOfLastPart3 greater than
     * pos_main_data_begin is wrong and a call to `scan_ancillary' may lead to reset (by
     * mistake) the enhanced feature bytes while detection of mp3pro/mp3surround streams
     * is ongoing.
     * This is likely to happen when sync goes lost and a bunch of frames have to be skipped
     */
    if (*endOfLastPart3 > pos_main_data_begin)
        *endOfLastPart3 = -1;

    if (pos_main_data_begin != -1  /* was the beginning of main_data found? */
        &&
        *endOfLastPart3 != -1)     /* was I able to seek the end of part2+3 data for the previous frame? */
    {
        int ret;
        if ((*endOfLastPart3)>0)
        {
            ret=seek_p23b(*endOfLastPart3);
            skip_p23b(1);
        }
        else
        {
            ret=seek_p23b(-1*(*endOfLastPart3));
        }
        if (ret)
        {
            int transferred, byte_off;
            /* the function `p23b_cpy' will take care in rounding `*endOfLastPart3'
             * to the next byte and when needed, move the bit pointer to the next block.
             */
            transferred = p23b_cpy(local_buff,pos_main_data_begin,local_buff_size, &byte_off);
            if (op_flag&OPERATIONAL_FLAG_DETECT_MC_IN_ALL_LAYERS)
                scan_multichannel(si, local_buff, byte_off, transferred*8, current_frame);
            if (byte_off > 0)
                /* fixing ancillary start, since useful detection data get stored byte-aligned */
                scan_ancillary_III(di,si,transferred-1,local_buff+1);
            else
                scan_ancillary_III(di,si,transferred,local_buff);
        }
    }
    *endOfLastPart3 = -1;

    /* this block skips part2+3 data and sets endOfLastPart3 for the next iteration */
    if (pos_main_data_begin != -1)
    {
        if (seek_p23b(pos_main_data_begin))
        {
            if (current_frame->part2_3_length) /* is there audio data actually? sometimes there isn't.
                                   Of course one can think this is due to data corruption,
                                   anyway I can't say anything without trying to decode data */
            {
                /* I will skip `part2_3_length-1' bits for a reason:
                 * skipping `part2_3_length' bits will make the bit pointer seek to the
                 * first bit of non-part2+3 data.
                 * In some cases, when a frame doesn't contain ancillary data at all,
                 * this bit happens to be on the next frame (in other words, the next
                 * array entry) which is not yet loaded into p23b and performing the
                 * seek can lead to unexpected results.
                 */
                if (skip_p23b(current_frame->part2_3_length-1) == 0)
                {
                    *endOfLastPart3 = tell_p23b(); /* now `endoflastpart3' points to the very last bit of part2+3 */
                    things_went_ok=1;
                }
                else
                {
                    /* not enough data found for the seek-forward operation.
                     * this may be due to either a broken frame (which doesn't contain
                     * enough data) or data corruption (which caused a wrong value into
                     * part2_3_length)
                     */
                    if (verbosity&VERBOSE_FLAG_PROGRESS)
                    {
                        if (current_frame->uncertain)
                            printf("  This frame is broken.\n\n");
                        else
                            printf("Frame %u may be corrupted.\n",si->totFrameNum);
                    }
                }
            }
            else
            {
                *endOfLastPart3 = -1*tell_p23b(); /* set a noticeable value for `endOfLastPart3' ! */
                if (current_frame->uncertain && (verbosity&VERBOSE_FLAG_PROGRESS))
                /* this last frame is broken and it has part2_3_length=0 => I think it's corrupted */
                    printf("  This frame is corrupted.\n\n");
            }
        }
        else
        {
            /* This is a very uncommon case: we cannot seek to the beginning of main
             * data block, although we can calculate it. For this to happen, the frame
             * has to have main_data_begin=0 so no backpointer (part2+3 begin into
             * the frame itself, after the side information block) and the frame has
             * to be broken at the end of the side information block (or before).
             */
            if (verbosity&VERBOSE_FLAG_PROGRESS)
            {
                if (current_frame->uncertain)
                    printf("  ");
                printf("Audio data block is missing at all.\n\n");
            }
        }
    }
    /* tell the user about the last frame */
    if (current_frame->uncertain && things_went_ok)
    {
        current_frame->uncertain=0;
        if (verbosity&VERBOSE_FLAG_PROGRESS)
            printf("  This frame is valid.\n\n");
    }
}

void scan(FILE *input_file, off_t *pos, streamInfo *si, detectionInfo *di, off_t id3pos, int verbosity, unsigned char op_flag)
{
    unsigned int head,head_pattern,mask;
    off_t oldpos;
    unsigned char local_buff[LARGEST_FRAME];
    unsigned char *buf;
    /* `endOfLastPart3' is used in order to get started with the scan into ancillary data.
     * When it stores a positive number, then it points to the last bit of a part2+part3
     * audio data block. It is used by seeking at `endOfLastPart3' and then skipping a single
     * further bit (a non-audio-data bit is expected there).
     * The value -1 means `unset'.
     * Finally, any other negative number means an usable value. It is used by seeking at
     * `-endOfLastPart3' but this time no skipping gets performed. This is useful for frames
     * which don't have audio data block (part2+part3 length is zero), so in those cases
     * the pointed bit is already a non-audio-data bit and the seek to `-endOfLastPart3'
     * does not need a subsequent bit skip.
     */
    bitoffs_t endOfLastPart3=-1;
    currentFrame current_frame;

/*
 * I'm sure I'm about to read the first valid audio frame of the stream,
 * since `scan' is called right after a successfull resync_mpeg().
 * Now resync_mpeg() is called again in order to get the value of `head'
 * and to properly set the file pointer.
 */

    oldpos=resync_mpeg(input_file, *pos, &buf, NULL, 0);
    head = extract_bits(buf, NULL, 32);

/*
 * Gathering basic properties of the mpeg stream, from now on I will assume
 * the other frames have the same mpeg version/layer/freq/etc...
 */

    si->freeformat  = (head&HEADER_FIELD_BITRATE)==BITRATE_INDEX_FREEFORMAT;
    si->lsf         = 1-((head&HEADER_FIELD_LSF)    >> HEADER_FIELD_LSF_SHIFT);
    di->id          = (head&HEADER_FIELD_MPEG_ID)   >> HEADER_FIELD_MPEG_ID_SHIFT;    /* 0=mpeg2.5, 1=resvd, 2=mpeg2, 3=mpeg1 */
    di->freq        = samplerate_tab[(head&HEADER_FIELD_SAMPRATE) >> HEADER_FIELD_SAMPRATE_SHIFT][di->id];
    di->lay         = 4-((head&HEADER_FIELD_LAYER)  >> HEADER_FIELD_LAYER_SHIFT);    /* so it is 1=layerI, 2=layerII, 3=layerIII */
    di->mode        = (head&HEADER_FIELD_CHANNELS)  >> HEADER_FIELD_CHANNELS_SHIFT;  /* channel mode (stereo, Jstereo, Dchannel, mono) */

    si->crc_present = ((~head)&HEADER_FIELD_CRC)    >> HEADER_FIELD_CRC_SHIFT;
    si->crc_checked = 1;
    si->copyright   = (head&HEADER_FIELD_COPYRIGHT) >> HEADER_FIELD_COPYRIGHT_SHIFT;
    si->original    = (head&HEADER_FIELD_ORIGINAL)  >> HEADER_FIELD_ORIGINAL_SHIFT;
    si->emphasis    = (head&HEADER_FIELD_EMPHASIS)  >> HEADER_FIELD_EMPHASIS_SHIFT;

    setup_framesize(head, si, di);

    /* mask selection: looks like layer I/II when encoding jstereo, switch between different stereo
       modes (they put 0x0, 0x1 or 0x2 in the channels field), while layer III only uses
       jstereo frames (0x1 in the channels field) and puts further information into the mode
       extension field */
    if (layerIII(head)||((head&HEADER_FIELD_CHANNELS>>HEADER_FIELD_CHANNELS_SHIFT)==MODE_MONO))
      mask=HEADER_CONSTANT_FIELDS_STRICT;
    else
      mask=HEADER_CONSTANT_FIELDS;

    /* using a bit mask will let me check a whole header in a single `if' statement */
    head_pattern = head & mask;


    memset((void *)&current_frame, 0, sizeof(currentFrame));
    current_frame.min_global_gain[GLOBAL_GAIN_CHANNEL_LEFT]=current_frame.min_global_gain[GLOBAL_GAIN_CHANNEL_RIGHT]=255;
    current_frame.maybe_garbage=1;

    while (*pos!=-1)
    {
        head = extract_bits(buf, NULL, 32);

        current_frame.maybe_garbage=0; /* when *pos!=-1 I am sure a valid header was found */

        if (*pos!=oldpos)
        {
            if (verbosity&VERBOSE_FLAG_PROGRESS)
                printf("Resync done at frame %u, skipped %d bytes.\n",si->totFrameNum,(int)(*pos-oldpos));
            endOfLastPart3=-1;
        }


        current_frame.bitrate_index=(head&HEADER_FIELD_BITRATE)>>HEADER_FIELD_BITRATE_SHIFT;

        if (head&HEADER_FIELD_PADDING)
            current_frame.expected_framesize=si->padded[(int)current_frame.bitrate_index].frameSize;
        else
            current_frame.expected_framesize=si->unpadded[(int)current_frame.bitrate_index].frameSize;

        /* now check the header we've just read */
        if ((head&mask)!=head_pattern || (freeformat(head)!=si->freeformat))
        {
            si->nSyncError++;
            if (verbosity&VERBOSE_FLAG_PROGRESS)
                printf("Sync error at frame %u, offset %lld. Seeking for resync...\n",si->totFrameNum,(long long)*pos);
            *pos=resync_mpeg(input_file, (*pos)+1, &buf, si, 0);
            continue;
        }

        /* checks on the very first part of the frame */
        if (malformed_part1(buf, &current_frame))
        {
            si->nSyncError++;
            if (verbosity&VERBOSE_FLAG_PROGRESS)
                printf("Corrupted frame %u, offset %lld. Seeking for resync...\n",si->totFrameNum,(long long)*pos);
            *pos=resync_mpeg(input_file, (*pos)+1, &buf, si, 0);
            continue;
        }

        /* check integrity for this frame */
        if (*pos+(off_t)current_frame.expected_framesize>id3pos)
        {
            if (*pos+(off_t)((current_frame.part1_length-1)/8+1) <= id3pos)
            {
                /* this frame has not the size we expected, maybe it's truncated.
                   part1 seems to be there, nevertheless warn the user and take care */
                if (verbosity&VERBOSE_FLAG_PROGRESS)
                    printf("Frame %u at offset %lld appears to be incomplete.\n  %d bytes available, %d expected.\n",si->totFrameNum,(long long)*pos,(int)(id3pos-(*pos)),current_frame.expected_framesize);
                current_frame.uncertain=1;
                current_frame.expected_framesize=(int)(id3pos-(*pos)); /* fixing framesize */
            }
            else
            {
                /* cannot read the very first part of the frame - surely broken */
                if (verbosity&VERBOSE_FLAG_PROGRESS)
                    printf("Frame %u at offset %lld is broken.\n  %d bytes available, %d expected.\n\n",si->totFrameNum,(long long)*pos,(int)(id3pos-(*pos)),current_frame.expected_framesize);
                endOfLastPart3 = -1;  /* invalidate the search for ancillary - it makes no sense without a whole part1 information */
                break;
            }
        }


        if (!(head&HEADER_FIELD_CRC))
        {
            /* verify the crc */
            current_frame.crc = extract_bits(buf+sizeof(unsigned int), NULL, 16);

            if (!verify_crc(head, buf+sizeof(unsigned int)+sizeof(unsigned short), current_frame.part1_length, current_frame.crc))
            {
                si->crc_checked = 0;
                if (verbosity&VERBOSE_FLAG_PROGRESS)
                    printf("CRC mismatch at frame %u, offset %lld. Resync...\n",si->totFrameNum,(long long)*pos);
                *pos=resync_mpeg(input_file,(*pos)+1, &buf, si, 0);
                continue;
            }
        }

        if (layerI(head))
        {
            scan_layer__I(si, di, &current_frame, buf, verbosity, op_flag);
        }

        if (layerII(head))
        {
            scan_layer_II(si, di, &current_frame, buf, verbosity);
        }

        if (layerIII(head))
        {
            scan_layerIII(si,di,(bitoffs_t)(*pos)*8,&current_frame,&endOfLastPart3,buf,local_buff,LARGEST_FRAME,verbosity,op_flag);
        }

        if (current_frame.uncertain)
            break;

        /* Now I am sure the frame is valid */

        if (head&HEADER_FIELD_PADDING)
        {
            di->usesPadding = 1;
            si->padded[(int)current_frame.bitrate_index].frameCount++;
        }
        else
            si->unpadded[(int)current_frame.bitrate_index].frameCount++;
        si->bitrateCount[(int)current_frame.bitrate_index]++;

        si->totFrameLen += (off_t)current_frame.expected_framesize;
        si->totFrameNum++;

        if (layerIII(head))   /* skip useless computations in layers other than III - just a speed-up */
        {
            if (current_frame.main_data_begin > si->reservoirMax)
                si->reservoirMax=current_frame.main_data_begin;
            di->usesScfsi=di->usesScfsi | current_frame.usesScfsi[0] | current_frame.usesScfsi[1];
            di->usesScalefacScale=di->usesScalefacScale | current_frame.usesScalefacScale;

            if (current_frame.min_global_gain[GLOBAL_GAIN_CHANNEL_LEFT] < si->min_global_gain[GLOBAL_GAIN_CHANNEL_LEFT])
                si->min_global_gain[GLOBAL_GAIN_CHANNEL_LEFT]=current_frame.min_global_gain[GLOBAL_GAIN_CHANNEL_LEFT];
            if (current_frame.min_global_gain[GLOBAL_GAIN_CHANNEL_RIGHT] < si->min_global_gain[GLOBAL_GAIN_CHANNEL_RIGHT])
                si->min_global_gain[GLOBAL_GAIN_CHANNEL_RIGHT]=current_frame.min_global_gain[GLOBAL_GAIN_CHANNEL_RIGHT];
            if (current_frame.max_global_gain[GLOBAL_GAIN_CHANNEL_LEFT] > si->max_global_gain[GLOBAL_GAIN_CHANNEL_LEFT])
                si->max_global_gain[GLOBAL_GAIN_CHANNEL_LEFT]=current_frame.max_global_gain[GLOBAL_GAIN_CHANNEL_LEFT];
            if (current_frame.max_global_gain[GLOBAL_GAIN_CHANNEL_RIGHT] > si->max_global_gain[GLOBAL_GAIN_CHANNEL_RIGHT])
                si->max_global_gain[GLOBAL_GAIN_CHANNEL_RIGHT]=current_frame.max_global_gain[GLOBAL_GAIN_CHANNEL_RIGHT];

            di->blockCount[BLOCKCOUNT_LONG]  +=current_frame.blockCount[BLOCKCOUNT_LONG];
            di->blockCount[BLOCKCOUNT_SHORT] +=current_frame.blockCount[BLOCKCOUNT_SHORT];
            di->blockCount[BLOCKCOUNT_MIXED] +=current_frame.blockCount[BLOCKCOUNT_MIXED];
            di->blockCount[BLOCKCOUNT_SWITCH]+=current_frame.blockCount[BLOCKCOUNT_SWITCH];

            if (op_flag&OPERATIONAL_FLAG_VERIFY_MUSIC_CRC)
            {
                si->musicCRC = crc_reflected_update(si->musicCRC, buf, current_frame.expected_framesize);
            }
        }

        if ((head&HEADER_FIELD_CHANNELS)>>HEADER_FIELD_CHANNELS_SHIFT == MODE_JOINT_STEREO)
        {
            /* if jstereo, then update mode_extension data */
            di->modeCount[(head&HEADER_FIELD_MODEXT)>>HEADER_FIELD_MODEXT_SHIFT]++;
            /* when handling layer I/II we may find a stereo frame at the beginning, jstereo frames
               (signaling the file is encoded as jstereo) may be encountered later */
            di->mode=MODE_JOINT_STEREO;
        }
        else
            di->modeCount[4]++;

        *pos += (off_t)current_frame.expected_framesize;
        oldpos = *pos;

        if (*pos == id3pos)
            /* end of stream reached */
            break;

        *pos=resync_mpeg(input_file,*pos, &buf, si, 0);

        memset((void *)&current_frame, 0, sizeof(currentFrame));
        current_frame.min_global_gain[GLOBAL_GAIN_CHANNEL_LEFT]=current_frame.min_global_gain[GLOBAL_GAIN_CHANNEL_RIGHT]=255;
        current_frame.maybe_garbage=1;

    } /* END OF `while' CYCLE */

    if (layerIII(head))
    {
        int transferred=0;   /* amount of ancillary bytes */
        if (endOfLastPart3 != -1)
        {
            int ret;
            bitoffs_t end_of_last_frame;
            seek_p23b((bitoffs_t)oldpos*8-1); /* replaced startOfFrame with oldpos */
            end_of_last_frame = tell_p23b()+1;
            if (endOfLastPart3>0)
            {
                ret=seek_p23b(endOfLastPart3);
                skip_p23b(1);
            }
            else
            {
                ret=seek_p23b(-1*endOfLastPart3);
            }

            if (ret)
            {
                int byte_off;
                /* we'll leave the rounding part to `p23b_cpy' */
                transferred = p23b_cpy(local_buff,end_of_last_frame,LARGEST_BUFFER,&byte_off);
                if (op_flag&OPERATIONAL_FLAG_DETECT_MC_IN_ALL_LAYERS)
                    scan_multichannel(si, local_buff, byte_off, transferred*8, &current_frame);
                if (byte_off > 0)
                {
                    /* fixing ancillary start, since useful detection data get stored byte-aligned */
                    scan_ancillary_III(di,si,--transferred,local_buff+1);
                }
                else
                    scan_ancillary_III(di,si,transferred,local_buff);
            }
        }
        /* check whether ancillary data only hides into the very last frame - useful for Blade detection
         * It is still valid where there aren't ancillary bytes at all */
        di->ancillaryOnlyIntoLastFrame = (di->ancillaryData == (off_t)transferred);
    }
    /* `oldpos' now stores the last `to-be-checked' byte offset, is it exactly the same as id3pos? */
    if (oldpos<id3pos && current_frame.maybe_garbage)
    {
        if (checkmm_partial_tag(input_file, oldpos, &musicmatch_tag))
        {
            /* this probably is a broken musicmatch tag */
            if (verbosity&VERBOSE_FLAG_METADATA)
                printf("Partial or broken MusicMatch tag found: %u bytes long, (offset 0x%08X).\nSoftware ver %s, Xing encoder ver %s.\n\n",
                       (int)(id3pos-oldpos),(unsigned)oldpos,musicmatch_tag.mm_ver,musicmatch_tag.enc_ver);
        }
        else
        {
            if (verbosity&VERBOSE_FLAG_PROGRESS)
                printf("Unexpected data at %lld (length in bytes: %d).\n\n",(long long)oldpos,(int)(id3pos-oldpos));
        }
    }
}

/*
 ******************      END SCAN      ******************
 */

char check_bit_allocation_I(unsigned int header, unsigned char *buff, currentFrame *frame_desc)
{
/*
 * this routine is heavily based upon the source code of libmad (layer12.c).
 */
    char errors = 0;
    unsigned char bitalloc;
    unsigned int pointer = 0, bound, nch, ch, sb;

    if (((header&HEADER_FIELD_CHANNELS)>>HEADER_FIELD_CHANNELS_SHIFT) == MODE_JOINT_STEREO)
        bound = 4 + ((header&HEADER_FIELD_MODEXT)>>HEADER_FIELD_MODEXT_SHIFT) * 4;
    else
        bound = 32;

    nch = (((header&HEADER_FIELD_CHANNELS)>>HEADER_FIELD_CHANNELS_SHIFT) == MODE_MONO)? 1 : 2;

    for (sb=0; sb<bound; sb++)
    {
        for (ch=0; ch<nch; ch++)
        {
            bitalloc = extract_bits(buff, &pointer, 4);
            if (bitalloc == 15)
            {
                errors++;
                break;
            }
            else
            {
                if (frame_desc != NULL)
                {
                    if (bitalloc)
                        frame_desc->allocation[ch][sb] = bitalloc + 1;
                    else
                        frame_desc->allocation[ch][sb] = bitalloc;
                }
            }
        }
        /* speed-up in case of errors */
        if (ch != nch)
            break;
    }
    if (sb == bound)
    {
        /* everything went ok up to 'bound' */
        for (; sb<32; sb++)
        {
            bitalloc = extract_bits(buff, &pointer, 4);
            if (bitalloc == 15)
            {
                errors++;
                break;
            }
            else
            {
                if (frame_desc != NULL)
                {
                    if (bitalloc)
                        frame_desc->allocation[0][sb] = frame_desc->allocation[1][sb] = bitalloc + 1;
                    else
                        frame_desc->allocation[0][sb] = frame_desc->allocation[1][sb] = bitalloc;
                }
            }
        }
    }

    if (frame_desc != NULL)
    {
        frame_desc->bound = bound;
        frame_desc->part1_length = pointer + 8*(sizeof(unsigned int)+
                                   ((header&HEADER_FIELD_CRC)? 0 : sizeof(unsigned short)));
    }

    return errors;
}

char save_bit_allocation_scfsi_II(unsigned int header, unsigned char *buff, currentFrame *frame_desc)
{
/*
 * this routine is heavily based upon the source code of libmad (layer12.c).
 */
    unsigned char index, sblimit, bound, idx, jdx, nch;
    unsigned short nbit, samplerate, bitrate_perchannel;
    unsigned int pointer;

    if (frame_desc != NULL)
    {
        /* layer II needs the most complex process in order
           to identify how many bits will be checksummed */

        nch = (((header&HEADER_FIELD_CHANNELS) >> HEADER_FIELD_CHANNELS_SHIFT)==MODE_MONO) ? 1 : 2;
        samplerate = samplerate_tab[(header&HEADER_FIELD_SAMPRATE) >> HEADER_FIELD_SAMPRATE_SHIFT]
                                   [(header&HEADER_FIELD_MPEG_ID)  >> HEADER_FIELD_MPEG_ID_SHIFT];
        bitrate_perchannel = bitrate_tab[1-((header&HEADER_FIELD_LSF)   >> HEADER_FIELD_LSF_SHIFT)]
                                        [3-((header&HEADER_FIELD_LAYER) >> HEADER_FIELD_LAYER_SHIFT)]
                                        [(header&HEADER_FIELD_BITRATE)  >> HEADER_FIELD_BITRATE_SHIFT]
                                        / nch;

        if (((header&HEADER_FIELD_LSF)>>HEADER_FIELD_LSF_SHIFT)==0)
        {
            /* mpeg2 */
            index = 4;
        }
        else
        {
            /* mpeg1 */
            if (((header&HEADER_FIELD_BITRATE)>>HEADER_FIELD_BITRATE_SHIFT) == BITRATE_INDEX_FREEFORMAT)
            {
                if (samplerate == 48000)
                    index = 0;
                else
                    index = 1;
            }
            else
            {
                if (bitrate_perchannel <= 48)
                {
                    if (samplerate == 32000)
                        index = 3;
                    else
                        index = 2;
                }
                else
                {
                    if (bitrate_perchannel <= 80)
                        index = 0;
                    else
                    {
                        if (samplerate == 48000)
                            index = 0;
                        else
                            index = 1;
                    }
                }
            }
        }

        sblimit = subband_limit[index];
        if (((header&HEADER_FIELD_CHANNELS)>>HEADER_FIELD_CHANNELS_SHIFT) == MODE_JOINT_STEREO)
            bound = 4 + ((header&HEADER_FIELD_MODEXT)>>HEADER_FIELD_MODEXT_SHIFT) * 4;
        else
            bound = 30;
        if (bound > sblimit)
            bound = sblimit;

        /* decode bit allocations */
        pointer = 0;
        for (idx=0; idx<bound; idx++)
        {
            for (jdx=0; jdx<nch; jdx++)
            {
                nbit = subband_quanttable[index][idx];
                frame_desc->allocation[jdx][idx]=extract_bits(buff, &pointer, nbit);
            }
        }
        for (; idx<sblimit; idx++)
        {
            nbit = subband_quanttable[index][idx];
            frame_desc->allocation[0][idx]=
            frame_desc->allocation[1][idx]=extract_bits(buff, &pointer, nbit);
        }

        /* each scalefactor selection info entry is 2 bit long */
        for (idx=0; idx<sblimit; idx++)
            for (jdx=0; jdx<nch; jdx++)
                if (frame_desc->allocation[jdx][idx])
                    frame_desc->scfsi[jdx][idx]=extract_bits(buff, &pointer, 2);

        /* keep useful information */
        frame_desc->index = index;
        frame_desc->bound = bound;
        frame_desc->part1_length = pointer+8*(sizeof(unsigned int)+
                                   ((header&HEADER_FIELD_CRC)? 0 : sizeof(unsigned short)));
    }

    return 0;
}

char malformed_part1(unsigned char *buffer, currentFrame *frame_desc)
{
    char result=0;
    int header=extract_bits(buffer, NULL, 32);
    int offset=sizeof(unsigned int)+((header&HEADER_FIELD_CRC)? 0 : sizeof(unsigned short));

    if (layerIII(header))
    {
        result = read_side_information(header, buffer+offset, frame_desc);
    }
    else
    {
        if (layerI(header))
        {
            result = check_bit_allocation_I(header, buffer+offset, frame_desc);
        }
        else
        {
            /* layer II has no reserved values in bit-allocation info block */
            /* Nevertheless, it is useful gathering some information for later use */
            result = save_bit_allocation_scfsi_II(header, buffer+offset, frame_desc);
        }
    }

    return result;
}


/* This routine prints lots of detailed information about the processed mpeg bitstream */
void show_stream_info(streamInfo *si, detectionInfo *di, vbrtagdata_t *vbrTag, int verbosity, unsigned char op_flgs)
{
    int idx;
    char max_len,blk_max_len,dummy_string[12];
    float duration,sum;

    max_len=sprintf(dummy_string,"%u",si->totFrameNum);
    duration=(float)si->totFrameNum*(float)samples_tab[si->lsf][di->lay-1]/(float)di->freq;

    if (verbosity&VERBOSE_FLAG_VBR_TAG)
    {
        if (di->ofl)
        {
            printf("Original File Length block found.\n");
            printf("  Encoder delay            : %u samples\n", si->ofl_encDelay);
            printf("  Length of original audio : %u samples\n\n", si->ofl_orig_samples);
        }
    }

    if (verbosity&VERBOSE_FLAG_STREAM_DETAILS)
    {
        printf("Detected MPEG stream version %s layer ",!(di->id)? "2.5" : (di->id==MPEG_CODE_MPEG2)? "2" : "1");
        for (idx=0; idx<di->lay; idx++) printf("I");
        printf(", details follow.\n  File size               : %lld bytes\n",(long long)si->filesize);
        printf("  Audio stream size       : %lld bytes",(long long)si->totFrameLen);
        /* if the file has a header tag, then show the total size */
        if (vbrTag->infoTag!=TAG_NOTAG)
        {
            printf (" (including tag: %lld)",(long long)(si->totFrameLen+(off_t)vbrTag->frameSize));
            /* just a simple check */
            if ((si->totFrameLen+(off_t)vbrTag->frameSize) > si->filesize)
                printf(" (!!!)");
        }
        else
            if (si->totFrameLen > si->filesize)
                printf(" (!!!)");

        printf("\n  Length                  : %u:%02u:%02u.%03.0f",((int)duration/3600),(((int)duration%3600)/60),((int)duration%60),(1000*(duration-(int)duration)));
        if (duration >= 60.0)
            printf(" (%1.3f seconds)",duration);
        printf("\n  Data rate               : ");
        printf("%1.1f kbps",(float)si->totFrameLen*(float)di->freq*0.008/((float)si->totFrameNum*(float)samples_tab[si->lsf][di->lay-1]));
        if (si->freeformat) printf(" (free format bitstream)");
        printf("\n  Number of frames        : %u",si->totFrameNum);
        //max_len=printf("%d",si.totFrameNum);
        if (di->lay==3)
            printf("\n  Blocks per frame        : %d (granules per frame %d, channels per granule %d)",((di->id==MPEG_CODE_MPEG1)?2:1)*((di->mode==MODE_MONO)?1:2),(di->id==MPEG_CODE_MPEG1)?2:1,(di->mode==MODE_MONO)?1:2);
        printf("\n  Audio samples per frame : %d\n",samples_tab[si->lsf][di->lay-1]);
        printf("  Audio frequency         : %d Hz\n",di->freq);
//        if (si->encDelay!=-1)
//            printf("  Encoder delay            : %d samples\n",si->encDelay);
        //if (di.lameTag)
//        if (di->lame_tag && vbrTag->encDelay!=-1)
//        {
//            si->orig_samples=si->totFrameNum*samples_tab[si->lsf][di->lay-1]-vbrTag->encDelay-vbrTag->encPadding;
//            si->orig_samples=vbrTag->reported_frames*samples_tab[si->lsf][di->lay-1]-vbrTag->encDelay-vbrTag->encPadding;
//        }
//        if (si->orig_samples!=-1)
//            printf("  Length of original audio : %d samples\n",si->orig_samples);
        printf("  Encoding mode           : ");
        switch (di->mode)
        {
            case MODE_MONO:
                printf("mono");
                break;
            case MODE_DUAL_CHANNEL:
                printf("dual channel");
                break;
            case MODE_JOINT_STEREO:
                printf("joint stereo");
                break;
            case MODE_PLAIN_STEREO:
                printf("stereo");
                break;
        }
        if (di->lay==3)
        {
            printf("\n  Min global gain         : ");
            if (di->mode==MODE_MONO) /* Mono? */
                printf("c=%3d",si->min_global_gain[GLOBAL_GAIN_CHANNEL_MONO]);
            else
                printf("l=%3d  r=%3d",si->min_global_gain[GLOBAL_GAIN_CHANNEL_LEFT],si->min_global_gain[GLOBAL_GAIN_CHANNEL_RIGHT]);
            printf("\n  Max global gain         : ");
            if (di->mode==MODE_MONO) /* Mono? */
                printf("c=%3d",si->max_global_gain[GLOBAL_GAIN_CHANNEL_MONO]);
            else
                printf("l=%3d  r=%3d",si->max_global_gain[GLOBAL_GAIN_CHANNEL_LEFT],si->max_global_gain[GLOBAL_GAIN_CHANNEL_RIGHT]);
        }
        if (
            ((di->lay==2 && (si->mc.mc_stream_verified&1)) ||
             (di->lay==2 && si->mc_coherent_frames>si->totFrameNum*19/20) ||
             (di->lay!=2 && si->mc.mc_stream_verified))
            && (si->mc_verified_frames+si->mc_coherent_frames)>(si->totFrameNum*3/10)
           )
        {
            unsigned char ch=((di->mode==MODE_MONO)?1:2);
            printf("\n  Multi channel stream    : ");
            if (si->mc.mc_stream_verified == 1)
                printf("yes");
            else
            {
                /* MSb is set! So, there are some coherent frames, although they're not validated */
                if ((si->mc_verified_frames+si->mc_coherent_frames) < si->totFrameNum)
                    printf("maybe (%d%% probability)",(si->mc_verified_frames+si->mc_coherent_frames)*100/si->totFrameNum);
                else
                    printf("almost sure (99%% probability)");
            }
            printf("\nMPEG-2 Audio Multichannel parameters.\n  Audio bitstream extension      : %s\n", (si->mc.extension)?"yes":"no");
            printf("  LFE                            : %s\n", (si->mc.lfe)?"yes":"no");
            printf("  Channels                       : %d\n", si->mc.mc_channels);
            printf("  Multi lingual channels         : %d\n", si->mc.multi_lingual);
            if (si->mc.multi_lingual > 0)
            {
                printf("  Multi lingual stream layer     : I%c\n", (si->mc.multi_lingual_layer)?' ':'I');
                printf("  Multi lingual stream frequency : %d\n", (si->mc.multi_lingual_fs)?di->freq/2:di->freq);
            }
            printf("  Channel configuration          : ");
            if ((si->mc.configuration_value&0xf0) == 0x30)
            {
                /* second stereo program */
//                printf("%d/0 and %d/0", ((si->mc.configuration_value&1)?ch+1:ch),
//                    ((si->mc.configuration_value&1)?si->mc.mc_channels-1:si->mc.mc_channels));
                printf("%d/0 and %d/0", ch+(si->mc.configuration_value&1),
                    si->mc.mc_channels-(si->mc.configuration_value&1));
            }
            else
            {
//                printf("%d/%d", ((si->mc.configuration_value&1)?ch+1:ch),
//                    ((si->mc.configuration_value&1)?si->mc.mc_channels-1:si->mc.mc_channels));
                printf("%d/%d", ch+(si->mc.configuration_value&1),
                    si->mc.mc_channels-(si->mc.configuration_value&1));
            }
        }
        printf("\nFlags\n  Error protection : %s", si->crc_present ? "yes":"no");
        if (si->crc_present)
            printf(" (check %s)", (si->crc_checked)?"passed":"failed");
        printf("\n  Copyrighted      : %s\n",si->copyright ? "yes":"no");
        printf("  Original         : %s\n",si->original ? "yes":"no");
        printf("  Emphasis         : %s\n\n",si->emphasis == 0 ? "none" : si->emphasis == 1 ? "50/15ms" : "CCITT");

        if (di->mode==MODE_JOINT_STEREO) /* Jstereo? */
        {
            sum=(float)(di->modeCount[0]+di->modeCount[1]+di->modeCount[2]+di->modeCount[3]+di->modeCount[4]);
            if (di->lay==3)
            {
                printf("Mode extension: stereo mode frame count\n");
                if (di->modeCount[0]) printf("  Simple stereo                 : %*d (%4.1f%%)\n",max_len,di->modeCount[0],(float)di->modeCount[0]*100.0/sum);
                if (di->modeCount[1]) printf("  Intensity stereo              : %*d (%4.1f%%)\n",max_len,di->modeCount[1],(float)di->modeCount[1]*100.0/sum);
                if (di->modeCount[2]) printf("  Mid-side stereo               : %*d (%4.1f%%)\n",max_len,di->modeCount[2],(float)di->modeCount[2]*100.0/sum);
                if (di->modeCount[3]) printf("  Intensity and mid-side stereo : %*d (%4.1f%%)\n",max_len,di->modeCount[3],(float)di->modeCount[3]*100.0/sum);
                printf("  -----------------------------\n  sum                           ");
            }
            else
            {
                printf("Mode extension: intensity stereo applied in\n");
                if (di->modeCount[0]) printf("  Bands  4 to 31     : %*d (%4.1f%%)\n",max_len,di->modeCount[0],(float)di->modeCount[0]*100.0/sum);
                if (di->modeCount[1]) printf("  Bands  8 to 31     : %*d (%4.1f%%)\n",max_len,di->modeCount[1],(float)di->modeCount[1]*100.0/sum);
                if (di->modeCount[2]) printf("  Bands 12 to 31     : %*d (%4.1f%%)\n",max_len,di->modeCount[2],(float)di->modeCount[2]*100.0/sum);
                if (di->modeCount[3]) printf("  Bands 16 to 31     : %*d (%4.1f%%)\n",max_len,di->modeCount[3],(float)di->modeCount[3]*100.0/sum);
                if (di->modeCount[4]) printf("Simple stereo frames : %*d (%4.1f%%)\n",max_len,di->modeCount[4],(float)di->modeCount[4]*100.0/sum);
                printf("  ------------------\n  sum                ");
            }
            printf(": %d\n\n",(int)sum);
        }

        if (di->lay==3)
        {
            sum=(float)(di->blockCount[BLOCKCOUNT_LONG]+di->blockCount[BLOCKCOUNT_SHORT]+di->blockCount[BLOCKCOUNT_MIXED]+di->blockCount[BLOCKCOUNT_SWITCH]);
            /*
             * It is not hard to say that blk_max_len and max_len are related, since
             * granule number is related to frame number. Granule number may be the same
             * value as frame number if single channel and one granule per channel (mpeg2/2.5).
             * It may be frame number times two if either two channels and one granule per
             * channel or single channel and two granules per channel (mpeg1).
             * At last, it may be the frame number times four when two channels and two
             * granules per channel are used.
             * So blk_max_len may require at most a single digit more than max_len.
             */
            blk_max_len=max_len+1;
            printf("Block usage\n");
            if (di->blockCount[BLOCKCOUNT_LONG])   printf("  Long block granules   : %*d (%4.1f%%)\n",blk_max_len,di->blockCount[BLOCKCOUNT_LONG],  (float)di->blockCount[BLOCKCOUNT_LONG]*100.0/sum);
            if (di->blockCount[BLOCKCOUNT_MIXED])  printf("  Mixed block granules  : %*d (%4.1f%%)\n",blk_max_len,di->blockCount[BLOCKCOUNT_MIXED], (float)di->blockCount[BLOCKCOUNT_MIXED]*100.0/sum);
            if (di->blockCount[BLOCKCOUNT_SWITCH]) printf("  Switch block granules : %*d (%4.1f%%)\n",blk_max_len,di->blockCount[BLOCKCOUNT_SWITCH],(float)di->blockCount[BLOCKCOUNT_SWITCH]*100.0/sum);
            if (di->blockCount[BLOCKCOUNT_SHORT])  printf("  Short block granules  : %*d (%4.1f%%)\n",blk_max_len,di->blockCount[BLOCKCOUNT_SHORT], (float)di->blockCount[BLOCKCOUNT_SHORT]*100.0/sum);
            printf("  ---------------------\n  sum                   : %*d\n\n",blk_max_len,(int)sum);
        }
    }

    if (verbosity&VERBOSE_FLAG_ADVANCED_BITS)
    {
        printf("Ancillary data\n  Total amount   : %lld bytes (%3.1f%%)\n  Bitrate        : %1.1f kbps\n",(long long)di->ancillaryData,(float)di->ancillaryData/(float)si->totFrameLen*100.0,(duration!=0.0)?(float)di->ancillaryData*0.008/duration:0);

        if (di->ancillaryData)
            printf("  Min packet     : %d bytes\n  Max packet     : %d bytes\n",si->ancill_min,si->ancill_max);
        if (di->lay==3)
        {
            printf("Max reservoir                          : %d bytes\n",si->reservoirMax);
            printf("Scalefactor scaling used               : %s\n",(di->usesScalefacScale)?"yes":"no");
            printf("Scalefactor selection information used : %s\n",(di->usesScfsi>0)?"yes":"no");
        }
    }
    if (verbosity&VERBOSE_FLAG_ADVANCED_BITS)
        printf("Padding used     %s: %s\n\n",(di->lay==3)?"                      ":"",(di->usesPadding)?"yes":"no");

    if (verbosity&VERBOSE_FLAG_HISTOGRAM)
    {
        printf("Frame histogram\n");
        for(idx=0; idx<15; idx++)
            if (si->bitrateCount[idx])
            {
                if (idx==0)
                    printf("Custom frames : ");
                else
                    printf("%4d kbps : ",bitrate_tab[si->lsf][di->lay-1][idx]);
                printf("%*d (%4.1f%%), size distr: [",max_len,si->bitrateCount[idx],(float)si->bitrateCount[idx]*100.0/(float)si->totFrameNum);

                if (si->unpadded[idx].frameCount)
                    printf("%*d x%4d B",max_len,si->unpadded[idx].frameCount,si->unpadded[idx].frameSize);

                if ((si->unpadded[idx].frameCount)&&(si->padded[idx].frameCount))
                    printf(", ");

                if (si->padded[idx].frameCount)
                    printf("%*d x%4d B",max_len,si->padded[idx].frameCount,si->padded[idx].frameSize);

                printf("]\n");
            }
        printf("\n");
    }

    if (verbosity&VERBOSE_FLAG_PROGRESS)
        printf(" %d header errors.\n\n",si->nSyncError);

    if (op_flgs&OPERATIONAL_FLAG_VERIFY_MUSIC_CRC)
    {
        printf("Music CRC verification ");
        if (vbrTag->lameMusicCRC != -1)
            printf("%s\n\n", (si->musicCRC==vbrTag->lameMusicCRC)?"passed":"failed");
        else
            printf("not performed.\n\n");
    }

    if (di->encoder_string[0] != 0 && (verbosity&VERBOSE_FLAG_ENC_STRING))
        printf("Encoder string : %s\n\n",di->encoder_string);
}

/*
 * vbr detection
 * returns 0 if cbr encoding
 * otherwise (vbr) nonzero
 */
char detect_vbr_stream(streamInfo *si)
{
    int i,j=0;
    for (i=1; i<15; i++)
    {
        if (si->bitrateCount[i])
            j++;
    }
    return (j>1);
}

int guesser(detectionInfo *di)
{
    int ret;
/*
 * Note: Xing and helix encoders count in their xing tag also the xing tag itself
 * as a valid frame, resulting in a +1 difference with actual audio frames amount.
 * You have the same behaviour with FhG encoders and their VBRI tag
 */
    if (di->encoder_string[0]=='G' || di->encoder_string[0]=='L')
    {
        if (di->encoder_string[0]=='G')
            ret = ENCODER_GUESS_GOGO;
        else
            ret = ENCODER_GUESS_LAME;
    }
    else
    {
        if (di->ancillaryData   /* both the enhancements require more info, hidden into ancillary data */
            &&
            (
            (di->id!=MPEG_CODE_MPEG1 && (di->enhSignature[0]&PRO_SIGN_BYTE1)==PRO_SIGN_BYTE1 && (di->enhSignature[1]&PRO_SIGN_BYTE2)==PRO_SIGN_BYTE2)     /* case of mp3PRO */
            ||
            (di->id==MPEG_CODE_MPEG1 && (di->freq==44100 || di->freq==48000) && (di->enhSignature[0]&SURR_SIGN_BYTE1)==SURR_SIGN_BYTE1 && (di->enhSignature[1]&SURR_SIGN_BYTE2)==SURR_SIGN_BYTE2) /* case of mp3Surround */
            )
           )
        {
            if (di->id!=MPEG_CODE_MPEG1)
            {
                if (di->ofl)
                    ret = ENCODER_GUESS_MP3PRO_OFL;
                else
                    ret = ENCODER_GUESS_MP3PRO;
            }
            else
            {
                if (di->ofl)
                    ret = ENCODER_GUESS_MP3SURR_OFL;
                else
                    ret = ENCODER_GUESS_MP3SURR;
            }
        }
        else
        {
            if (di->vbr_tag==TAG_VBRITAG)
            {
                /* presence of VBRI tag means we're dealing with a vbr stream produced by a fraunhofer encoder */
                if (di->ofl)
                    /* mp3sEncoder is the encoder for mp3surround but it can encode both mono and stereo streams as well */
                    ret = ENCODER_GUESS_MP3S_ENC;
                else
                {
                    if (di->usesPadding)
                        ret = ENCODER_GUESS_FHG_MAYBE_FASTENC; /* both l3enc and mp3enc cannot encode vbr streams */
                    else
                        ret = ENCODER_GUESS_FHG_ACM;
                }
            }
            else
            {
                /* "...the Xing encoder never uses short blocks." */
                if (di->blockCount[BLOCKCOUNT_SHORT] == 0)
                {  /* helix encoder (which is based on xing source code) DOES use short blocks! */
                    if (di->modeCount[1])   /* intensity stereo encoded frames */
                        ret = ENCODER_GUESS_XING_VERY_OLD;
                    else
                    {
                        if (di->usesScfsi>0)
                            ret = ENCODER_GUESS_XING_NEW;
                        else
                            ret = ENCODER_GUESS_XING_OLD;
                    }
                }
                else
                {
                    /*
                     * BladeEnc does NOT use scalefactor scaling. Instead, scalefactor selection information
                     * is sometimes used. It cannot create VBR files and it rarely adds ancillary data bits.
                     * When it does, it is only into the very last frame (and puts in only 0xFF bytes).
                     * Padding is always enabled. Further, it encodes just either Mono or Plain Stereo
                     * mode (no joint/dual).
                     * Oh yes, it's very VERY easy to add support for a further encoder when it's open
                     * source software :-)
                     * MusicMatch tag presence is checked in order to exclude bladeenc, since MM encoder
                     * was xing, not blade
                     */
                    if (di->ancillaryOnlyIntoLastFrame && !di->usesScalefacScale && !di->vbr_stream && (di->mode==MODE_PLAIN_STEREO || di->mode==MODE_MONO) && !di->mm_tag)
                        ret = ENCODER_GUESS_BLADE;
                    else
                    {
                        if (di->usesScfsi>0)
                        {  /* gogo-no-coda (?) seems to be a japanese fork of lame using asm-optimizations in
                              order to provide max encoding speed; it adds NO lame tag and a custom "GOGO"
                              string in ancillary bits. Further, it adds a xing tag in vbr encodings only. */
                            if (di->usesScalefacScale)
                            {   /* the encoder may be lame as well as gogo as helix -- xing tag may help now */
                                if (di->vbr_tag==TAG_XINGTAG || di->vbr_tag==TAG_LAMECBRTAG)
                                {
                                    if (di->lame_tag)
                                        ret = ENCODER_GUESS_LAME;   /* ScaleFactor scaling is used in Lame since version 3.84 beta on */
                                    else
                                    {
                                        if (di->frameCountDiffers)
                                            ret = ENCODER_GUESS_HELIX;
                                        else
                                            ret = ENCODER_GUESS_GOGO;
                                    }
                                }
                                else
                                    ret = ENCODER_GUESS_HELIX;
                            }
                            else
                                ret = ENCODER_GUESS_LAME_OLD; /* Lame (up to) v3.83 beta didn't use it */
                        }
                        else
                        {
                            if (di->usesScalefacScale)
                            {
                                if (di->ofl)
                                    ret = ENCODER_GUESS_MP3S_ENC;
                                else
                                {
                                    if (di->vbr_stream)
                                        ret = ENCODER_GUESS_FHG_FASTENC;
                                    else
                                    {
                                        if (di->usesPadding)
                                            ret = ENCODER_GUESS_FHG_MAYBE_L3ENC;
                                        else
                                            ret = ENCODER_GUESS_FHG_ACM;
                                    }
                                }
                            }
                            else
                            {
                                if (di->mm_tag)
                                    ret = ENCODER_GUESS_XING_NEW;
                                else
                                    ret = ENCODER_GUESS_UNKNOWN;
                            }
                        }
                    }
                }
            }
        }
    }
    return ret;
}



void print_version(char verbose)
{
    printf ("%s-%u.%u",VER_PKG_NAME,VER_MAJ,VER_MIN);
    if (VER_PATCH_LEV!=0) printf(".%u",VER_PATCH_LEV);
#ifdef __MINGW32__
    printf("-%d.%d.%d", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#endif
    if (RELEASE_STABLE)   printf(" stable");
    if (RELEASE_ALPHA)    printf(" alpha");
    if (RELEASE_BETA)     printf(" beta");
    if (RELEASE_PREVIEW)  printf(" preview");
    if (RELEASE_RC)       printf(" RC");
#ifdef REL_SUBNUM
    if (REL_SUBNUM!=1)    printf (" %u", REL_SUBNUM);
#endif
    if (verbose)
    {
#ifdef CODENAME
        printf(", \"%s\".",CODENAME);
#endif

        printf("\nLarge file support: %s\n",(sizeof(off_t)==SIZEOF_OFF64_T)?"yes":"no");
        printf("Executable built on %s.", __DATE__);
    }
    printf("\n");
}

void usage()
{
    print_version(0);
    printf("Show detailed information about mpeg audio files and guess the encoder used.\n"
           "Usage:\t%s [ [OPTIONS] filename ] | [ -h ] | [ -V ]\n"
           "Options:\n"
           "-- controlling output\n"
           "  mp3guessenc can be very verbose when reporting scan results. Use the following\n"
           "  options to select what you want/don't want to see. The default is to show\n"
           "  everything, the guessed encoder name cannot be silenced.\n"
           "  -a\tshow advanced bit usage (ancillary, bit reservoir, scalefactor, padding)\n"
           "  -e\tshow everything (default)\n"
           "  -f\tshow frame histogram\n"
           "  -g\tshow the message `Maybe this file is encoded by...'\n"
           "  -i\tshow metadata info tags\n"
           "  -m\tshow mpeg stream details\n"
           "  -n\tshow nothing at all -- just the encoder name will be reported\n"
           "  -p\tshow progress messages\n"
           "  -s\twhen found, show encoder string\n"
           "  -v\tshow VBR info tag\n"
           "-- operational\n"
           "  -c\tforce multi channel detection in all layers (default layerII only)\n"
           "  -h\tprint this help screen\n"
           "  -r\tverify music crc when lame tag is found (expensive!)\n"
#ifdef ENABLE_SKIP_OPTION
           "  -S n\tseek to a specified offset n before starting analysis\n"
#endif
           "  -V\tprint version information\n"
           "\n",VER_PKG_NAME);
}


/* main */
int mp3guessenc_timing_shift_case(const char *file)
//int mp3guessenc_main(int argc,char **argv)
{
    char verify_failed=0, ape_header;
    id3tag id3;
    unsigned char id3V2Maj, id3V2min;
    unsigned char *buf;
    int /*option,*/ id3v1size, id3v2size, apetagsize, apetagvers, apetagitems,
        verbose_flags, lyrics3v1size, lyrics3v2size, wavechunk, datachunk;
    unsigned char operational_flags=0;
#ifdef ENABLE_SKIP_OPTION
    int force_skip=-1;
#endif
    unsigned int head;
    off_t id3pos=0, pos;
    FILE *input_file;
    detectionInfo di;
    streamInfo si;
    vbrtagdata_t TagData;
    int return_value = EXIT_CODE_CASE_D;
#ifdef CODENAME
//    char source[DATA_LEN];
//    int idx;
#endif
#ifdef _WIN32
    struct _stati64 fileStat;
    #define stat _stati64
#else
    struct stat fileStat;
#endif /* _WIN32 */
    head_metadata_tag_t t_found=HEAD_NO_TAG;
    off_t first_byte=-1, riff_at=-1;
    int current_storage=0, start_search;

    /* reset some values */
    memset((void *)&di, 0, sizeof(detectionInfo));
    memset((void *)&si, 0, sizeof(streamInfo));

    di.enhSignature[0]=di.enhSignature[1]=255;
    si.ancill_min=LARGEST_BUFFER;
//    si.encDelay=si.orig_samples=-1;
    si.min_global_gain[GLOBAL_GAIN_CHANNEL_LEFT]=si.min_global_gain[GLOBAL_GAIN_CHANNEL_RIGHT]=255;
    //verbose_flags = VERBOSE_FLAG_SHOW_EVERYTHING | VERBOSE_FLAG_UNTOUCHED;
    verbose_flags = VERBOSE_FLAG_VBR_TAG;

    /* is `input_file' a regular file? */
//    if (stat(argv[optind],&fileStat))
    if (stat(file,&fileStat))
    {
        printf("Error getting file attributes for `%s' (does it exist?), exiting.\n",file);
        return EXIT_CODE_STAT_ERROR;
    }
/*    
    if (
        !S_ISREG(fileStat.st_mode)
#ifndef _WIN32
        && !S_ISLNK(fileStat.st_mode)
#endif
       )
    {
        printf("File `%s' is not a regular file, unable to proceed, exiting.\n",file);
        return EXIT_CODE_INVALID_FILE;
    }
*/    
    if (fileStat.st_size == 0)
    {
        printf("File `%s' is zero bytes long, nothing to scan, exiting.\n",file);
        return EXIT_CODE_INVALID_FILE;
    }

    /*
     * Will `bitoffs_t' data type be large enough for
     * storage of bit pointers?
     * I ask for `bitoffs_t' to be 64 bit and `off_t' may either
     * be 32-bit or 64-bit long.
     * sizeof(bitoffs_t)>sizeof(off_t) is OK for me, and I never expect
     * it to be smaller. What if they are the same?
     */
    if (sizeof(bitoffs_t) == sizeof(off_t))
    {
        bitoffs_t dummy=(bitoffs_t)fileStat.st_size;
        if (
            dummy > dummy*2
            ||
            dummy > dummy*4
            ||
            dummy > dummy*8
           )
        {
            /* `bitoffs_t' data type has insufficient bits to handle this file */
            printf("File `%s' cannot be analyzed because its size is too large.\n",file);
            return EXIT_CODE_CANNOT_HANDLE;
        }
    }


    si.filesize = fileStat.st_size;

//    input_file = fopen(argv[optind],"rb");
    input_file = fopen(file,"rb");
/*    
    if (input_file == NULL)
    {
        printf("Fatal: error opening `%s'.\nDouble-check the provided file name or ensure you have the proper access rights.\n", argv[optind]);
        return EXIT_CODE_ACCESS_ERROR;
    }

    if (verbose_flags&VERBOSE_FLAG_PROGRESS)
        printf("Reading `%s'...\n",argv[optind]);
*/
    /* check if this file has metadata tags at its tail */
    id3pos = si.filesize;

    do
    {
        id3v1size = checkid3v1(input_file,id3pos,&id3);
        if (id3v1size)
        {
            id3pos -= (off_t)id3v1size;
            if (verbose_flags&VERBOSE_FLAG_METADATA)
            {
                printf("ID3tag v1.%c found (offset 0x%08X).\n",(!id3.comment[28]&&id3.comment[29])?'1':'0',(unsigned)id3pos);
                show_id3v1(&id3);
            }
        }

        lyrics3v1size=checklyrics3v1(input_file,id3pos);
        if (lyrics3v1size)
        {
            id3pos -= lyrics3v1size;
            if (verbose_flags&VERBOSE_FLAG_METADATA)
                printf("Lyrics3v1 tag found, %d bytes long (offset 0x%08X).\n\n",lyrics3v1size,(unsigned)id3pos);
        }

        lyrics3v2size=checklyrics3v2(input_file,id3pos);
        if (lyrics3v2size)
        {
            id3pos -= lyrics3v2size;
            if (verbose_flags&VERBOSE_FLAG_METADATA)
                printf("Lyrics3v2.00 tag found, %d bytes long (offset 0x%08X).\n\n",lyrics3v2size,(unsigned)id3pos);
        }

        apetagsize=checkapetagx_tail(input_file,id3pos,&apetagvers,&apetagitems,&ape_header);
        if (apetagsize)
        {
            id3pos -= apetagsize;
            if (verbose_flags&VERBOSE_FLAG_METADATA)
                printf("APE tag v%c found at the end of the stream (offset 0x%08X).\n%d items, %d bytes long, %s.\n\n",
                       (apetagvers==2000)?'2':'1',(unsigned)id3pos,apetagitems,apetagsize,
                       (ape_header)?"optional header present":"no header");
        }

        memset((void *)&musicmatch_tag, 0, sizeof(mmtag_t));
        checkmmtag(input_file, id3pos, &musicmatch_tag);
        if (musicmatch_tag.tag_size)
        {
            di.mm_tag = 1;
            id3pos -= musicmatch_tag.tag_size;
            if (verbose_flags&VERBOSE_FLAG_METADATA)
            {
                printf("MusicMatch tag found: %u bytes long, metadata section is %u bytes long (offset 0x%08X).\nSoftware ver %s, tag ver %s, Xing encoder ver %s, %s.\n",
                       musicmatch_tag.tag_size,musicmatch_tag.metadata_size,(unsigned)id3pos,musicmatch_tag.mm_ver,musicmatch_tag.tag_ver,musicmatch_tag.enc_ver,
                       (musicmatch_tag.header_present)?"header present":"no header");
                if (musicmatch_tag.image_size)
                {
                    printf("Image detected at offset %llu, size %u, extension %s\n",(long long)musicmatch_tag.image_offset,musicmatch_tag.image_size,musicmatch_tag.image_ext);
                }
                printf("\n");
            }
        }

        id3v2size=checkid3v2_footer(input_file,id3pos,&id3V2Maj,&id3V2min);
        if (id3v2size>0)
        {
            id3pos -= (off_t)id3v2size;
            if (verbose_flags&VERBOSE_FLAG_METADATA)
            {
                /* an ID3v2 tag at the end should be revision 4 or newer */
                printf("ID3tag v2.%01u.%01u found at file tail (offset 0x%08X)\nID3tag v2 is %d bytes long.\n\n",id3V2Maj,id3V2min,(unsigned)id3pos,id3v2size);
            }
        }
        else
        {
            if (id3v2size==-1)
            {
                if (verbose_flags&VERBOSE_FLAG_METADATA)
                    printf("Errors detected in id3V2 tag footer.\n\n");
                /* I will check the size of the tag in order to remain into this loop */
                /* so in this case it has to be fixed */
                id3v2size=0;
            }
        }
    }
    while (id3v1size!=0 || lyrics3v1size!=0 || lyrics3v2size!=0 || apetagsize!=0 || musicmatch_tag.tag_size!=0 || id3v2size!=0);

    /* now check if this file has metadata tags at its head. */
    pos = 0;
#ifdef ENABLE_SKIP_OPTION
    if (force_skip != -1 && force_skip > 0)
        pos = (off_t)force_skip;
#endif


    do
    {
        int tag_pos, next_tag;

        if (t_found != HEAD_WAVERIFF_UNCOMPLETE_TAG)
            t_found = HEAD_NO_TAG;
        tag_pos = LARGEST_BUFFER;

        if (pos > (first_byte+(off_t)current_storage-HEAD_METADATA_MIN_IDENTIFICATION_LENGTH))
        {
            /* need to fill the buffer */
            fseeko(input_file, pos, SEEK_SET);
            first_byte = pos;
            current_storage = fread(mp3g_storage, 1, LARGEST_BUFFER, input_file);
        }

        start_search = (int)(pos - first_byte);

        if (t_found == HEAD_WAVERIFF_UNCOMPLETE_TAG)
        {
            /* here the search for subsequent chunks is performed */
            /* last one will be the `data' chunk */
            wavechunk=checkwaveriff_datachunk(&mp3g_storage[start_search],
                    &next_tag,&datachunk);
            if (wavechunk > 0)
            {
                if (datachunk > 0)
                {
                    t_found = HEAD_WAVERIFF_DATACHUNK_TAG;
                    tag_pos = next_tag;
                }
                else
                {
                    /* an other chunk found */
                    tag_pos = 0;
                }
            }
            else
            {
                /* no further chuck found, quite odd */
                /* maybe the file is corrupted, abort */
                t_found = HEAD_NO_TAG;
            }
        }
        else
        {
            /* here we seek for the very first RIFF-WAVE pattern */
            wavechunk=checkwaveriff(&mp3g_storage[start_search],
                    current_storage-start_search,&next_tag);
            if (wavechunk > 0)
            {
                if (next_tag < tag_pos)
                {
                    t_found = HEAD_WAVERIFF_UNCOMPLETE_TAG;
                    tag_pos = next_tag;
                    riff_at = pos + next_tag;
                }
            }
        }

        id3v2size = checkid3v2(&mp3g_storage[start_search],current_storage-start_search,
                &next_tag,&id3V2Maj,&id3V2min);
        if (id3v2size > 0)
        {
            if (next_tag < tag_pos)
            {
                t_found = HEAD_ID3V2_TAG;
                tag_pos = next_tag;
            }
        }

        apetagsize = checkapetagx_head(&mp3g_storage[start_search],current_storage-start_search,
                &next_tag,&apetagvers,&apetagitems,&ape_header);
        if (apetagsize > 0)
        {
            if (next_tag < tag_pos)
            {
                t_found = HEAD_APE_TAG;
                tag_pos = next_tag;
            }
        }

        /* detection complete, now we will print tag information, if any */

        if (t_found != HEAD_NO_TAG)
        {
            if (tag_pos)
                printf("Unexpected data at %lld (length in bytes: %d)\n\n", (long long)pos, tag_pos);

            switch (t_found)
            {
                case HEAD_ID3V2_TAG:
                if (verbose_flags&VERBOSE_FLAG_METADATA)
                {
                    printf("ID3tag v2.%01u.%01u found (offset 0x%08X).\n",id3V2Maj,id3V2min,(unsigned)pos+tag_pos);
                    printf("ID3tag v2 is %d bytes long, skipping...\n\n",id3v2size);
                }
                pos += (off_t)(tag_pos+id3v2size);
                break;

                case HEAD_APE_TAG:
                if (verbose_flags&VERBOSE_FLAG_METADATA)
                    printf("APE tag v%c found at the head of the stream (offset 0x%08X).\n%d items, %d bytes long, %s.\n\n",
                           (apetagvers==2000)?'2':'1',(unsigned)pos+tag_pos,apetagitems,apetagsize,
                           (ape_header)?"optional header present":"no header");
                pos += (off_t)(tag_pos+apetagsize);
                break;

                case HEAD_WAVERIFF_DATACHUNK_TAG:
                if (verbose_flags&VERBOSE_FLAG_METADATA)
                    printf("Wave Riff header found (offset 0x%08X), size %d bytes.\nData chunk is %d bytes long.\n\n",
                           (unsigned)riff_at,(int)(pos+tag_pos+wavechunk-riff_at),datachunk);
                pos += (off_t)(tag_pos+wavechunk);   /* here `tag_pos' should be zero */
                /* check whether to expect pad byte */
                if (datachunk&1)
                {
                    /* size is odd, so we expect to find a zero padding byte at the end */
                    if (pos+datachunk+1==id3pos)
                    {
                        /* id3pos is at the end of the chunk, not of the REAL data */
                        id3pos--;
                    }
                }
                break;

                case HEAD_WAVERIFF_UNCOMPLETE_TAG:
                pos += (off_t)(tag_pos+wavechunk);
                break;

                default:
                break;
            }
        }
        else
        {
            /* force update of the data buffer */
            current_storage = 0;
        }
    }
    while (t_found != HEAD_NO_TAG || start_search != 0);

    /* due to this `fseek', I will assure the file pointer points */
    /* at the beginning of the non-id3v2 stuff */

    fseeko(input_file,pos,SEEK_SET);
    /* trick: metadata total size into id3v2size */
    id3v2size = pos;

    pos--;

    /* find first frame */
    /*
     * lame encoder prior to version 3.94 beta (dec 15, 2003) used to set a wrong
     * bitrate index (1111b) in the info header for free format streams
     * I have to allow a more relaxed scan in order not to skip a valid info header
     */

    do
    {
        currentFrame local;
        unsigned short target;
        off_t pos_new;
        unsigned int head_new, header_mask;

        memset((void *)&local, 0, sizeof(currentFrame));
        pos = resync_mpeg(input_file, pos+1, &buf, NULL, 1);
        if (pos != -1)
        {
            if ((pos-id3v2size) > SCAN_SIZE)
            {
                /* too far - not allowed, set result of a failed search */
                pos = -1;
                break;
            }

            head = extract_bits(buf, NULL, 32);
            if (malformed_part1(buf, &local))
            {
                verify_failed = 1;
                continue;
            }

            if (!(head&HEADER_FIELD_CRC))
            {
                verify_failed = 1;
                /* verify the crc */
                target = extract_bits(buf+sizeof(unsigned int), NULL, 16);
                if (!verify_crc(head, buf+sizeof(unsigned int)+sizeof(unsigned short), local.part1_length, target))
                {
                    /* not a valid frame - skip! */
                    continue;
                }
            }


            /* first of all, check for a vbr tag
               this will work on any layerIII stream, even when we're dealing with
               a freeformat stream created by an ancient lame enc */

            /* search for the xing/lame/vbri tag */
            if (checkvbrinfotag(&TagData,buf,pos,di.encoder_string))
            {
                /* vbr tag found - this means the mpeg frame is valid
                   eventually, checkvbrinfotag fixed the bitrate index into the mpeg header */
                head = TagData.header;
            }
            else 
            {
                return_value = EXIT_CODE_CASE_A;                                          
            }

            if (((head&HEADER_FIELD_LAYER)>>HEADER_FIELD_LAYER_SHIFT)==LAYER_CODE_L_III &&
                ((head&HEADER_FIELD_BITRATE)>>HEADER_FIELD_BITRATE_SHIFT)==BITRATE_INDEX_RESERVED)
            {
                /* now the header should have been fixed - sure this is junk, not audio data */
                verify_failed = 1;
                continue;
            }

            /* now, seek for the next valid frame and calculate the frame size */
            pos_new = pos;
            if (layerIII(head))
            {
                pos_new += local.part1_length/8;
                /* the 'local' struct was filled in by the 'malformed_part1' function */
                if (local.part2_3_length/8 - local.main_data_begin > 0)
                    pos_new += local.part2_3_length/8 - local.main_data_begin;
                pos_new--; /* this is needed to compensate for the plus one below... */
            }
            else
            {
                /* layerI & layerII */
                pos_new += (local.part1_length+local.part2_3_length)/8 - 1;
            }

            verify_failed = 0;
            do
            {
                currentFrame candidate;
                memset((void *)&candidate, 0, sizeof(currentFrame));

                /* locate a new valid audio mpeg header */
                pos_new = resync_mpeg(input_file, pos_new+1, &buf, NULL, 0);
                if (pos_new != -1)
                {
                    if ((pos_new-pos) > SCAN_SIZE)
                    {
                        /* too far - not allowed */
                        verify_failed = 1;
                        break;
                    }

                    head_new = extract_bits(buf, NULL, 32);
                    if (malformed_part1(buf, &candidate))
                    {
                        continue;
                    }
                    else
                    {
                        /* valid part1 ! */
                        if (!(head_new&HEADER_FIELD_CRC))
                        {
                            /* verify the crc */
                            target = extract_bits(buf+sizeof(unsigned int), NULL, 16);
                            if (!verify_crc(head_new, buf+sizeof(unsigned int)+sizeof(unsigned short), candidate.part1_length, target))
                            {
                                /* not a valid frame - skip! */
                                continue;
                            }
                        }
                        /* no crc or valid crc */

                        /* select the right bitmask */
                        if (((head&HEADER_FIELD_CHANNELS)>>HEADER_FIELD_CHANNELS_SHIFT) == MODE_MONO)
                        {
                            header_mask = HEADER_CONSTANT_FIELDS_STRICT;
                        }
                        else
                        {
                            header_mask = HEADER_CONSTANT_FIELDS;
                        }
                        if (TagData.infoTag == TAG_VBRITAG)
                        {
                            /* the fake frame containing vbri tag always has the crc flag disabled,
                               regardless of the protection setting of the audio stream */
                            header_mask &= ~HEADER_FIELD_CRC;
                        }
                        if (TagData.infoTag == TAG_LAMECBRTAG || TagData.infoTag == TAG_XINGTAG)
                        {
                            /* some versions of lame used to set the 'original' flag to zero into the
                               very first frame (the lame tag) regardless of the general flag set
                               into the real stream */
                            header_mask &= ~HEADER_FIELD_ORIGINAL;
                        }

                        if ( (head_new&header_mask) == (head&header_mask) )
                        {
                            if (freeformat(head))
                            {
                                /* it's a valid frame and it's also coherent with our previous one */
                                if (get_bitrate(head, (int)(pos_new-pos)) <= BITRATE_MAX_ALLOWED_FREEFORMAT)
                                {
                                    int slot=(((head&HEADER_FIELD_LAYER)>>HEADER_FIELD_LAYER_SHIFT)==LAYER_CODE_L___I)?4:1;
                                    /* here the exact size of the first freeformat frame is evaluated */
                                    if (head&HEADER_FIELD_PADDING)
                                    {
                                        si.padded[0].frameSize=(int)(pos_new-pos);
                                        si.unpadded[0].frameSize=(int)(pos_new-pos)-slot;
                                    }
                                    else
                                    {
                                        si.unpadded[0].frameSize=(int)(pos_new-pos);
                                        si.padded[0].frameSize=(int)(pos_new-pos)+slot;
                                    }
                                    TagData.frameSize = (int)(pos_new-pos);
                                }
                                else
                                {
                                    /* frame size says we're dealing with bitrate > 640.0 kbps */
                                    /* this clearly is wrong */
                                    verify_failed = 1;
                                    break;
                                }
                            }
                            else
                            {
                                /* frame with 'normal' bitrate */
                                if ((pos_new-pos) != get_framesize(head)
                                    &&
                                    (head&HEADER_FIELD_EMPHASIS) != 0)
                                {
                                    /* frames have junk in between and some unusual emphasis set - not good */
                                    verify_failed = 1;
                                    break;
                                }
                                TagData.frameSize = get_framesize(head);
                            }

                            verify_failed = 0;
                            if (TagData.infoTag != TAG_NOTAG)
                            {
                                /* meaningful tag found - it is not a frame to be analyzed */
                                if (id3v2size != pos)
                                {
                                    printf("Unexpected %d bytes before VBR tag.\n", (int)(pos-id3v2size));
                                }

                                pos = pos_new;

                                if (verbose_flags&VERBOSE_FLAG_VBR_TAG)
                                {
                                    //show_info_tag(&TagData);
                                    return_value = check_timing_shift_case(&TagData);
 
                                    pos_new = -1;
                                    pos = -1;
                                }
                            }
                            break;
                        }
                        else
                        {
                            /* uncoherent frame headers */
                            continue;
                        }
                    }
                }
                else
                {
                    if (freeformat(head))
                    {
                        /* no more frames found - this can be troublesome for freeformat streams,
                           which require a frame size to be set anyway */
                        int slot=(((head&HEADER_FIELD_LAYER)>>HEADER_FIELD_LAYER_SHIFT)==LAYER_CODE_L___I)?4:1;
                        off_t dummy=id3pos;

                        while (get_bitrate(head, (int)(dummy-pos)) > BITRATE_MAX_ALLOWED_FREEFORMAT)
                            dummy--;
                        if (head&HEADER_FIELD_PADDING)
                        {
                            si.padded[0].frameSize=(int)(dummy-pos);
                            si.unpadded[0].frameSize=(int)(dummy-pos)-slot;
                        }
                        else
                        {
                            si.unpadded[0].frameSize=(int)(dummy-pos);
                            si.padded[0].frameSize=(int)(dummy-pos)+slot;
                        }
                        TagData.frameSize = (int)(dummy-pos);
                    }
                }
            }
            while (pos_new!=-1);
        }
    }
    while (pos!=-1
           &&
           verify_failed);

    if (pos == -1)
    {
        //printf("Cannot find valid mpeg header, scan failed.\n\n");
        fclose(input_file);
        //return EXIT_CODE_NO_MPEG_AUDIO;
        return return_value;
    }

    /* does the mpeg stream start right after the heading junk ? */
    if (pos!=id3v2size && TagData.infoTag == TAG_NOTAG)
        if (verbose_flags&VERBOSE_FLAG_PROGRESS)
            printf("Unexpected data at %d (length in bytes: %d).\n",id3v2size,(int)(pos-(off_t)id3v2size));


    /* undo latest modifications to 'buf' */
    resync_mpeg(input_file, pos, &buf, NULL, 1);

    if (TagData.infoTag != TAG_NOTAG)
    {
        di.vbr_tag=TagData.infoTag;
        di.lame_tag=(TagData.lametag[0]!=0);

        if (pos != (off_t)(TagData.tagStartsAt+TagData.frameSize))
        {
            /* some junk between the tag and the very first audio frame */
            if (verbose_flags&VERBOSE_FLAG_VBR_TAG)
                printf("Unexpected %d bytes between tag and the audio stream.\n",(int)(pos-(off_t)(TagData.tagStartsAt+TagData.frameSize)));
        }
    }

    if (verbose_flags&VERBOSE_FLAG_PROGRESS)
        printf("First frame found at %lld (0x%08X).\n\n",(long long)pos,(unsigned)pos);

    scan(input_file,&pos,&si,&di,id3pos,verbose_flags,
        /* performarce: do not calculate music crc when there is no valid crc sum */
        ((TagData.lameMusicCRC!=-1)?operational_flags:(operational_flags&~OPERATIONAL_FLAG_VERIFY_MUSIC_CRC)));

    fclose(input_file);
/*
    if (si.totFrameNum)
    {
        show_stream_info(&si,&di,&TagData,verbose_flags,operational_flags);

        if (di.vbr_tag!=TAG_NOTAG)
            di.frameCountDiffers=(si.totFrameNum!=TagData.reported_frames);

        di.vbr_stream=detect_vbr_stream(&si);

        if (di.lay==3)
        {
            int result;
            if (verbose_flags&VERBOSE_FLAG_GUESSING)
                printf("Maybe this file is encoded by ");
            result = guesser(&di);
            printf("%s\n\n", encoder_table[result]);
            return result;
        }
    }
    else
        if (verbose_flags&VERBOSEFLAG_PROGRESS)
            printf("No valid audio data.\n\n");
*/
    return return_value;
}
/*
int mp3guessenc_timing_shift_case(const char *file)
{
    char *argv[] = {"", "-v", (char*)file};
    return mp3guessenc_main(3, argv);
 }
 */