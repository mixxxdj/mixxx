/*
 *   mp3guessenc header
 *   Copyright (C) 2002-2010 Naoki Shibata
 *   Copyright (C) 2011-2018 Elio Blanca <eblanca76@users.sourceforge.net>
 *
 * Xing VBR tagging for LAME.
 * Copyright (c) 1999 A.L. Faber
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
/* Modifed by Evan Dekker 2019-09-26 */

#ifndef MP3GUESSENC_H
#define MP3GUESSENC_H

#include "tags.h"

#define EXIT_CODE_NO_ERROR                  0
#define EXIT_CODE_UNKNOWN_OPTION           -1
#define EXIT_CODE_STAT_ERROR               -2
#define EXIT_CODE_INVALID_FILE             -3
#define EXIT_CODE_ACCESS_ERROR             -4
#define EXIT_CODE_NO_MPEG_AUDIO            -5
#define EXIT_CODE_CANNOT_HANDLE            -6

#define EXIT_CODE_CASE_A                    1
#define EXIT_CODE_CASE_B                    2
#define EXIT_CODE_CASE_C                    3
#define EXIT_CODE_CASE_D                    4

#define HEADER_FIELD_SYNC          0xFFE00000
#define HEADER_FIELD_SYNC_SHIFT            21
#define HEADER_FIELD_MPEG_ID         0x180000
#define HEADER_FIELD_MPEG_ID_SHIFT         19
#define HEADER_FIELD_LSF              0x80000 /* this is not a *real* field, it is just a commodity for accessing lsf bit - 1:mpeg1, 0:mpeg2/2.5*/
#define HEADER_FIELD_LSF_SHIFT             19
#define HEADER_FIELD_LAYER            0x60000
#define HEADER_FIELD_LAYER_SHIFT           17
#define HEADER_FIELD_CRC              0x10000
#define HEADER_FIELD_CRC_SHIFT             16
#define HEADER_FIELD_BITRATE           0xF000
#define HEADER_FIELD_BITRATE_SHIFT         12
#define HEADER_FIELD_SAMPRATE           0xC00
#define HEADER_FIELD_SAMPRATE_SHIFT        10
#define HEADER_FIELD_PADDING            0x200
#define HEADER_FIELD_PADDING_SHIFT          9
#define HEADER_FIELD_PRIVATE            0x100
#define HEADER_FIELD_PRIVATE_SHIFT          8
#define HEADER_FIELD_CHANNELS            0xC0
#define HEADER_FIELD_CHANNELS_SHIFT         6
#define HEADER_FIELD_MODEXT              0x30
#define HEADER_FIELD_MODEXT_SHIFT           4
#define HEADER_FIELD_COPYRIGHT            0x8
#define HEADER_FIELD_COPYRIGHT_SHIFT        3
#define HEADER_FIELD_ORIGINAL             0x4
#define HEADER_FIELD_ORIGINAL_SHIFT         2
#define HEADER_FIELD_EMPHASIS             0x3
#define HEADER_FIELD_EMPHASIS_SHIFT         0

#define HEADER_CONSTANT_FIELDS_STRICT ( \
        HEADER_FIELD_SYNC |      \
        HEADER_FIELD_MPEG_ID |   \
        HEADER_FIELD_LAYER |     \
        HEADER_FIELD_CRC |       \
        HEADER_FIELD_CHANNELS |  \
        HEADER_FIELD_SAMPRATE |  \
        HEADER_FIELD_COPYRIGHT | \
        HEADER_FIELD_ORIGINAL |  \
        HEADER_FIELD_EMPHASIS)

#define HEADER_CONSTANT_FIELDS ( \
        HEADER_FIELD_SYNC |      \
        HEADER_FIELD_MPEG_ID |   \
        HEADER_FIELD_LAYER |     \
        HEADER_FIELD_CRC |       \
        HEADER_FIELD_SAMPRATE |  \
        HEADER_FIELD_COPYRIGHT | \
        HEADER_FIELD_ORIGINAL |  \
        HEADER_FIELD_EMPHASIS)

#define HEADER_ANY_BUT_BITRATE_AND_PADDING_FIELDS ( \
        HEADER_FIELD_SYNC |      \
        HEADER_FIELD_MPEG_ID |   \
        HEADER_FIELD_LAYER |     \
        HEADER_FIELD_CRC |       \
        HEADER_FIELD_SAMPRATE |  \
        HEADER_FIELD_PRIVATE |   \
        HEADER_FIELD_CHANNELS |  \
        HEADER_FIELD_MODEXT |    \
        HEADER_FIELD_COPYRIGHT | \
        HEADER_FIELD_ORIGINAL |  \
        HEADER_FIELD_EMPHASIS)


#define VERBOSE_FLAG_UNTOUCHED      0x00010000
#define VERBOSE_FLAG_GUESSING       0x00000001
#define VERBOSE_FLAG_ENC_STRING     0x00000002
#define VERBOSE_FLAG_HISTOGRAM      0x00000004
#define VERBOSE_FLAG_ADVANCED_BITS  0x00000008
#define VERBOSE_FLAG_STREAM_DETAILS 0x00000010
#define VERBOSE_FLAG_VBR_TAG        0x00000020
#define VERBOSE_FLAG_METADATA       0x00000040
#define VERBOSE_FLAG_PROGRESS       0x00000080

#define VERBOSE_FLAG_SHOW_EVERYTHING (VERBOSE_FLAG_GUESSING | \
                                      VERBOSE_FLAG_ENC_STRING | \
                                      VERBOSE_FLAG_HISTOGRAM | \
                                      VERBOSE_FLAG_ADVANCED_BITS | \
                                      VERBOSE_FLAG_STREAM_DETAILS | \
                                      VERBOSE_FLAG_VBR_TAG | \
                                      VERBOSE_FLAG_METADATA | \
                                      VERBOSE_FLAG_PROGRESS)

#define OPERATIONAL_FLAG_DETECT_MC_IN_ALL_LAYERS         0x80
#define OPERATIONAL_FLAG_VERIFY_MUSIC_CRC                0x40

#define SCAN_SIZE 65536  /* default scan length is 64 kB when searching for a valid frame */

#define LAYER_CODE_RESERVED      0
#define LAYER_CODE_L_III         1
#define LAYER_CODE_L__II         2
#define LAYER_CODE_L___I         3

#define BITRATE_INDEX_FREEFORMAT               0
#define BITRATE_INDEX_RESERVED                15

#define BITRATE_MAX_ALLOWED_FREEFORMAT     640.0

#define LARGEST_BUFFER                 (10*1024)   /* ensure this is larger than LARGEST_FRAME */


typedef struct detectionInfo {
    int     blockCount[4];
    int     modeCount[5];
    int     freq;
    off_t   ancillaryData;
    char    usesScfsi;         /* this must be set to 0 */
    char    usesScalefacScale; /* this must be set to 0 */
    char    usesPadding;       /* this must be set to 0 */
    char    vbr_stream;
    char    vbr_tag;
    char    lame_tag;
    char    mm_tag;
    char    frameCountDiffers;
    char    ancillaryOnlyIntoLastFrame;
    unsigned char   lay;       /* layer */
    unsigned char   id;        /* mpeg id (1/2/2.5) */
    unsigned char   mode;
    char    encoder_string[LAME_STRING_LENGTH];
/*
 * mp3PRO detection
 * ----------------
 * The mp3PRO detection method searchs for a "signature" the encoder puts into its audio data.
 * The encoder stores higher frequency information "hidden" into ancillary data of layerIII
 * streams while creating mpeg2 streams only. These infos are splitted into small packets (min
 * 6 bytes) and, given the first two bytes (big endian), they always show bits 15, 14 and 3 set.
 * Having said that, identification of the encoder is straightforward, since Fraunhofer IIS is
 * the patent owner and the only one providing encoders for mp3PRO.
 */

/*
 * The detection for mp3Surround (5.1 stream into a backward compatible mpeg1 layerIII stream)
 * works the same way, just the signature is obviously different.
 */
    unsigned char enhSignature[2]; /* these bytes of enhanced mp3 features must be initialized to 0xFF */
    char    ofl;
} detectionInfo;

typedef struct frameCounterCell {
    int frameSize;
    int frameCount;
} frameCounterCell;

typedef struct mpegMultiChannel {
    unsigned char mc_stream_verified;
    unsigned char extension;
    unsigned char lfe;
    unsigned char mc_channels;
    unsigned char multi_lingual;
    unsigned char multi_lingual_fs;
    unsigned char multi_lingual_layer;
    unsigned char configuration_value;
} mpegMultiChannel;

typedef struct streamInfo {
    off_t           filesize;
    off_t           totFrameLen;  /* this must be set to 0 */
    unsigned int    totFrameNum;  /* this must be set to 0 */
    unsigned char   lsf;
    char            crc_present;
    char            crc_checked;
    char            copyright;
    char            original;
    char            emphasis;
    /* the following two fields are written by recent fhg encoders and then hidden into
       ancillary bits - they differ from similar information taken from lame vbr tag */
    unsigned short  ofl_encDelay;
    unsigned int    ofl_orig_samples;
    int             reservoirMax; /* this must be set to 0 */
    int             bitrateCount[15];
    frameCounterCell    unpadded[15];
    frameCounterCell    padded[15];
    int             nSyncError;
    int             ancill_min;
    int             ancill_max;
    unsigned char   min_global_gain[2];
    unsigned char   max_global_gain[2];
    char            freeformat;   /* this must be set to 0 */
    mpegMultiChannel    mc;
    unsigned int    mc_verified_frames;
    unsigned int    mc_coherent_frames;
    unsigned short  musicCRC;
} streamInfo;

typedef struct currentFrame {
    char            maybe_garbage;
    char            uncertain;
    char            bitrate_index;
    char            usesScalefacScale;
    unsigned char   index;                 /* sub band quantization table index, used in layerII */
    char            bound;                 /* sub band limit, used in layerI & II */
    int             expected_framesize;    /* bytes */
    int             main_data_begin;       /* backpointer - bytes */
    char            usesScfsi[2];
    unsigned short  crc;
    unsigned char   min_global_gain[2];
    unsigned char   max_global_gain[2];
    int             blockCount[4];
    int             part1_length;          /* bits - length up to the last crc-covered bit */
    int             part2_3_length;        /* bits - remaining audio bits */
    unsigned char   allocation[5][32];     /* bit allocation data used in both layerI and II */
    unsigned char   scfsi[2][30];          /* scale factor selection info - used in layerII */
} currentFrame;


unsigned short crc_reflected_update(unsigned short, unsigned char *, unsigned int);
unsigned char reflect_byte(unsigned char);

#ifdef __cplusplus
extern "C" {
#endif
int mp3guessenc_timing_shift_case(const char *);    
#ifdef __cplusplus
}
#endif

#endif

