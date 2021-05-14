/*
 *   tags is a support module which provides access to metadata tags (and xing/vbri/lame tags as well)
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
/* Modifed by Evan Dekker 2019-09-26 */

#ifndef TAGS_H
#define TAGS_H

#if defined(__WINDOWS__)
#include <sys/types.h>
#endif

#define LAMETAGSIZE        36
#define LAME_STRING_LENGTH 48

#define TAG_NOTAG           0
#define TAG_VBRITAG         1
#define TAG_XINGTAG         2
#define TAG_LAMECBRTAG      3

/*
 * this value MUST be carefully choosen!
 * (or some calculation will be needed, so boring)
 * Current identification routines work for id3v2, apetag and wave riff,
 * (the metadata tags we can usually find at the very beginning)
 * and they require 10 bytes, 32 bytes and 12 bytes (just look at the code)
 * at least.
 * Should a new tag appear, this value will have to be evaluated again.
 */
#define HEAD_METADATA_MIN_IDENTIFICATION_LENGTH             32

typedef struct id3tag {
    unsigned char tag[3];
    unsigned char title[30];
    unsigned char artist[30];
    unsigned char album[30];
    unsigned char year[4];
    unsigned char comment[30];
    unsigned char genre;
} id3tag;

/* structure for MusicMatch tag */
typedef struct mmtag_t {
    unsigned int  tag_size;
    unsigned int  image_size;
    unsigned int  metadata_size;
    off_t         image_offset;
    char          mm_ver[5];   /* this field is actually four bytes long, this trick ensures there will always be a null string terminator */
    char          tag_ver[5];  /* this field is actually four bytes long, this trick ensures there will always be a null string terminator */
    char          enc_ver[5];  /* this field is actually four bytes long, this trick ensures there will always be a null string terminator */
    char          image_ext[5];/* this field is actually four bytes long, this trick ensures there will always be a null string terminator */
    char          header_present;
} mmtag_t;

/* structure to receive extracted header */
typedef struct vbrtagdata_t
{
    char    *tagId;
    off_t   tagStartsAt;
    unsigned int     header;           /* mpeg header of the frame containing the tag */
    int     frameSize;                 /* I want to keep the size of the frame containing the tag */
    char    infoTag;
    short   version;
    /* The following two fields come from lame vbr tag - they differ from similar info
       written by fhg encoders (stored into streamInfo structure) */
    short   encDelay;                  /* encoder delay (start) */
    short   encPadding;                /* encoder padding (samples added at the end of the wave) */
    short   tocEntries;
    short   sizePerTocEntry;
    short   framesPerTocEntry;
    int     tocSize;
    unsigned int reported_frames;      /* total bit stream frames from Vbr header data */
    unsigned int bytes;                /* total bit stream bytes from Vbr header data*/
    int     vbr_scale;                 /* encoded vbr scale from Vbr header data*/
    char    lame_buggy_vbrheader;
    unsigned char lametag[LAMETAGSIZE];
    char    lametagVerified;
    int     lameMusicCRC;                  /* this is actually a ushort
                                              but I need setting -1 for signaling empty value */
} vbrtagdata_t;


int  extract_enc_string(char *, unsigned char *, int);
char checkvbrinfotag(vbrtagdata_t *, unsigned char *, off_t, char *);
void show_info_tag (vbrtagdata_t *);
void show_id3v1(id3tag *);
int  checkid3v1(FILE *, off_t, id3tag *);
int  checkid3v2_footer(FILE *, off_t, unsigned char *, unsigned char *);
int  checkapetagx_tail(FILE *, off_t, int *, int *, char *);
int  checklyrics3v1(FILE *, off_t);
int  checklyrics3v2(FILE *, off_t);
void checkmmtag(FILE *, off_t, mmtag_t *);
char checkmm_partial_tag(FILE *, off_t, mmtag_t *);
int  checkid3v2(unsigned char *, int, int *, unsigned char *, unsigned char *);
int  checkapetagx_head(unsigned char *, int, int *, int *, int *, char *);
int  checkwaveriff(unsigned char *, int, int *);
int  checkwaveriff_datachunk(unsigned char *, int *, int *);

int check_timing_shift_case(vbrtagdata_t *);

#endif
