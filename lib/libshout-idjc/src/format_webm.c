/* -*- c-basic-offset: 8; -*- */
/* webm.c: WebM data handler
 * $Id$
 *
 *  Copyright (C) 2002-2012 the Icecast team <team@icecast.org>
 *  Copyright (C) 2015-2019 Philipp "ph3-der-loewe" Schafft <lion@lion.leolix.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
#   include <config.h>
#endif

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_INTTYPES_H
#   include <inttypes.h>
#endif

#include <shoutidjc/shout.h>
#include "shout_private.h"

/* -- local datatypes -- */

/* A value that no EBML var-int is allowed to take. */
#define EBML_UNKNOWN            ((uint64_t) -1)

/* masks to turn the tag ID varints from the Matroska spec
 * into their parsed-as-number equivalents */
#define EBML_LONG_MASK          (~0x10000000)
#define EBML_MID3_MASK          (~0x200000)
#define EBML_MID2_MASK          (~0x4000)
#define EBML_SHORT_MASK         (~0x80)

/* tag IDs we're interested in */
#define WEBM_EBML_ID            (0x1A45DFA3 & EBML_LONG_MASK)
#define WEBM_SEGMENT_ID         (0x18538067 & EBML_LONG_MASK)
#define WEBM_CLUSTER_ID         (0x1F43B675 & EBML_LONG_MASK)
#define WEBM_SEGMENT_INFO_ID    (0x1549A966 & EBML_LONG_MASK)

#define WEBM_TIMESTAMPSCALE_ID  (0x2AD7B1 & EBML_MID3_MASK)

#define WEBM_TIMECODE_ID        (0xE7 & EBML_SHORT_MASK)
#define WEBM_SIMPLE_BLOCK_ID    (0xA3 & EBML_SHORT_MASK)
#define WEBM_BLOCK_GROUP_ID     (0xA0 & EBML_SHORT_MASK)
#define WEBM_BLOCK_ID           (0xA1 & EBML_SHORT_MASK)

typedef enum webm_parsing_state {
    WEBM_STATE_READ_TAG = 0,
    WEBM_STATE_COPY_THRU
} webm_parsing_state;

/* state for a filter that extracts timestamp
 * information from a WebM stream
 */
/* TODO: provide for "fake chaining", where
 * concatinated files have extra headers stripped
 * and Cluster timestamps rewritten
 */
typedef struct _webm_t {

    /* processing state */
    bool waiting_for_more_input;
    webm_parsing_state parsing_state;
    uint64_t copy_len;

    /* buffer state */
    size_t input_write_position;
    size_t input_read_position;
    size_t output_position;

    /* Metadata */
    uint64_t timestamp_scale;

    /* statistics */
    uint64_t cluster_timestamp;
    uint64_t latest_timestamp;

    /* buffer storage */
    unsigned char input_buffer[SHOUT_BUFSIZE];
    unsigned char output_buffer[SHOUT_BUFSIZE];

} webm_t;

/* -- static prototypes -- */
static int  send_webm(shout_t *self, const unsigned char *data, size_t len);
static void close_webm(shout_t *self);

static int webm_process(shout_t *self, webm_t *webm);
static int webm_process_tag(shout_t *self, webm_t *webm);
static int webm_output(shout_t *self, webm_t *webm, const unsigned char *data, size_t len);

static size_t copy_possible(const void *src_base,
                            size_t *src_position,
                            size_t src_len,
                            void *target_base,
                            size_t *target_position,
                            size_t target_len);
static int flush_output(shout_t *self, webm_t *webm);

static ssize_t ebml_parse_tag(unsigned char *buffer,
                              unsigned char *buffer_end,
                              uint64_t *tag_id,
                              uint64_t *payload_length);
static ssize_t ebml_parse_var_int(unsigned char *buffer,
                                  unsigned char *buffer_end,
                                  uint64_t *out_value);
static ssize_t ebml_parse_sized_int(unsigned char       *buffer,
                                    unsigned char       *buffer_end,
                                    size_t              len,
                                    bool                 is_signed,
                                    uint64_t  *out_value);

/* -- interface functions -- */
int shout_open_webm(shout_t *self)
{
    webm_t *webm_filter;

    /* Alloc WebM filter */
    if (!(webm_filter = (webm_t *)calloc(1, sizeof(webm_t)))) {
        return self->error = SHOUTERR_MALLOC;
    }

    /* configure shout state */
    self->format_data = webm_filter;

    self->send = send_webm;
    self->close = close_webm;

    return SHOUTERR_SUCCESS;
}

static int send_webm(shout_t *self, const unsigned char *data, size_t len)
{
    webm_t *webm = (webm_t *) self->format_data;
    size_t input_progress = 0;

    self->error = SHOUTERR_SUCCESS;

    while (input_progress < len && self->error == SHOUTERR_SUCCESS) {
        copy_possible(data, &input_progress, len,
                      webm->input_buffer, &webm->input_write_position, SHOUT_BUFSIZE);

        self->error = webm_process(self, webm);
    }

    /* Squeeze out any possible output, unless we're failing */
    if (self->error == SHOUTERR_SUCCESS) {
        self->error = flush_output(self, webm);
    }

    /* Report latest known timecode for rate-control */
    self->senttime = (webm->latest_timestamp * webm->timestamp_scale) / 1000;

    return self->error;
}

static void close_webm(shout_t *self)
{
    webm_t *webm_filter = (webm_t *) self->format_data;

    if (webm_filter)
        free(webm_filter);
}

/* -- processing functions -- */

/* Process what we can of the input buffer,
 * extracting statistics or rewriting the
 * stream as necessary.
 * Returns a status code to indicate socket errors.
 */
static int webm_process(shout_t *self, webm_t *webm)
{
    size_t to_process;

    /* loop as long as buffer holds process-able data */
    webm->waiting_for_more_input = false;
    while (webm->input_read_position < webm->input_write_position
           && !webm->waiting_for_more_input
           && self->error == SHOUTERR_SUCCESS) {

        /* calculate max space an operation can work on */
        to_process = webm->input_write_position - webm->input_read_position;

        /* perform appropriate operation */
        switch (webm->parsing_state) {
            case WEBM_STATE_READ_TAG:
                self->error =  webm_process_tag(self, webm);
                break;

            case WEBM_STATE_COPY_THRU:
                /* copy a known quantity of bytes to the output */

                /* calculate size needing to be copied this step */
                if (webm->copy_len < to_process) {
                    to_process = webm->copy_len;
                }

                /* do copy */
                self->error = webm_output(self, webm,
                                          webm->input_buffer + webm->input_read_position,
                                          to_process);

                /* update state with copy progress */
                webm->copy_len -= to_process;
                webm->input_read_position += to_process;
                if (webm->copy_len == 0) {
                    webm->parsing_state = WEBM_STATE_READ_TAG;
                }

                break;

        }

    }

    if (webm->input_read_position < webm->input_write_position) {
        /* slide unprocessed data to front of buffer */
        to_process = webm->input_write_position - webm->input_read_position;
        memmove(webm->input_buffer, webm->input_buffer + webm->input_read_position, to_process);

        webm->input_read_position = 0;
        webm->input_write_position = to_process;
    } else {
        /* subtract read position instead of zeroing;
         * this allows skipping over large spans of data by
         * setting the read pointer far ahead. Processing won't
         * resume until the read pointer is actually within the buffer.
         */
        webm->input_read_position -= webm->input_write_position;
        webm->input_write_position = 0;
    }

    return self->error;
}

/* Try to read a tag header & handle it appropriately.
 * Returns an error code for socket errors or malformed input.
 */
static int webm_process_tag(shout_t *self, webm_t *webm)
{
    ssize_t tag_length;
    uint64_t tag_id;
    uint64_t payload_length;

    uint64_t timecode;
    ssize_t track_number_length;
    uint64_t track_number;
    uint64_t timestamp_scale;

    uint64_t to_copy;

    ssize_t status;

    unsigned char *start_of_buffer = webm->input_buffer + webm->input_read_position;
    unsigned char *end_of_buffer = webm->input_buffer + webm->input_write_position;

    /* parse tag header */
    tag_length = ebml_parse_tag(start_of_buffer, end_of_buffer, &tag_id, &payload_length);
    if (tag_length == 0) {
        webm->waiting_for_more_input = true;
        return self->error;
    } else if (tag_length < 0) {
        return self->error = SHOUTERR_INSANE;
    }

    /* most tags will be copied, header & payload, to output unaltered */
    to_copy = tag_length + payload_length;

    /* break open tags of unknown length, to process all children */
    if (payload_length == EBML_UNKNOWN) {
        to_copy = tag_length;
    }

    /* handle tag appropriately */

    switch (tag_id) {
        case WEBM_SEGMENT_ID:
        case WEBM_CLUSTER_ID:
            /* open containers to process children */
            to_copy = tag_length;
            break;

        case WEBM_SEGMENT_INFO_ID:
            /* open containers to process children */
            to_copy = tag_length;
            /* set defaults */
            webm->timestamp_scale = 1000000;
            break;

        case WEBM_TIMESTAMPSCALE_ID:
            /* read cluster timecode */
            status = ebml_parse_sized_int(start_of_buffer + tag_length,
                                          end_of_buffer,
                                          payload_length,
                                          false, &timestamp_scale);

            if (status == 0) {
                webm->waiting_for_more_input = true;
                return self->error;
            } else if (status < 0) {
                return self->error = SHOUTERR_INSANE;
            }

            webm->timestamp_scale = timestamp_scale;
            break;

        case WEBM_TIMECODE_ID:
            /* read cluster timecode */
            status = ebml_parse_sized_int(start_of_buffer + tag_length,
                                          end_of_buffer,
                                          payload_length,
                                          false, &timecode);

            if (status == 0) {
                webm->waiting_for_more_input = true;
                return self->error;
            } else if (status < 0) {
                return self->error = SHOUTERR_INSANE;
            }

            /* report timecode */
            webm->cluster_timestamp = timecode;
            webm->latest_timestamp = timecode;

            /* TODO: detect backwards jumps and rewrite to be monotonic */
            break;

        case WEBM_BLOCK_GROUP_ID:
            /* open container to process children */
            to_copy = tag_length;

            break;

        case WEBM_SIMPLE_BLOCK_ID:
        case WEBM_BLOCK_ID:
            /* extract block or simple block timecode */

            /* [simple] blocks start with a varint, so read it to
             * know the offset of the following fields
             */
            track_number_length = ebml_parse_var_int(start_of_buffer + tag_length,
                                                     end_of_buffer, &track_number);
            if (track_number_length == 0) {
                webm->waiting_for_more_input = true;
                return self->error;
            } else if (track_number_length < 0) {
                return self->error = SHOUTERR_INSANE;
            }

            /* now read the actual (signed 16-bit) timecode;
             * this code is relative to the Cluster's timecode.
             *
             * ASSUMPTION: it will not actually be negative,
             * since WebM encoding guidelines advise all timestamps
             * be monotonically increasing.
             */
            status = ebml_parse_sized_int(start_of_buffer + tag_length + track_number_length,
                                          end_of_buffer, 2, true, &timecode);

            if (status == 0) {
                webm->waiting_for_more_input = true;
                return self->error;
            } else if (status < 0) {
                return self->error = SHOUTERR_INSANE;
            }

            /* report timecode */
            webm->latest_timestamp = webm->cluster_timestamp + timecode;

            break;
    }

    /* queue copying */

    if (to_copy > 0) {
        webm->copy_len = to_copy;
        webm->parsing_state = WEBM_STATE_COPY_THRU;
    }

    return self->error;
}

/* Queue the given data in the output buffer,
 * flushing as needed. Returns a status code
 * to allow detecting socket errors on a flush.
 */
static int webm_output(shout_t *self, webm_t *webm, const unsigned char *data, size_t len)
{
    size_t output_progress = 0;

    while (output_progress < len && self->error == SHOUTERR_SUCCESS)
    {
        copy_possible(data, &output_progress, len,
                      webm->output_buffer, &webm->output_position, SHOUT_BUFSIZE);

        if (webm->output_position == SHOUT_BUFSIZE) {
            self->error = flush_output(self, webm);
        }
    }

    return self->error;
}

/* -- utility functions -- */

/* Copies as much of the source buffer into the target
 * as will fit, and returns the actual size copied.
 * Updates position pointers to match.
 */
static size_t copy_possible(const void *src_base,
                            size_t *src_position,
                            size_t src_len,
                            void *target_base,
                            size_t *target_position,
                            size_t target_len)
{
    size_t src_space = src_len - *src_position;
    size_t target_space = target_len - *target_position;
    size_t to_copy = src_space;

    if (target_space < to_copy) to_copy = target_space;

    memcpy((char*)target_base + *target_position, (char*)src_base + *src_position, to_copy);

    *src_position += to_copy;
    *target_position += to_copy;

    return to_copy;
}

/* Send currently buffered output to the server.
 * Output buffering is needed because parsing
 * and/or rewriting code may pass through small
 * chunks at a time, and we don't want to expend a
 * syscall on each one.
 * However, we do not want to leave sendable data
 * in the buffer before we return to the client and
 * potentially sleep, so this is called before
 * send_webm() returns.
 */
static int flush_output(shout_t *self, webm_t *webm)
{
    ssize_t ret;

    if (webm->output_position == 0) {
        return self->error;
    }

    ret = shout_send_raw(self, webm->output_buffer, webm->output_position);
    if (ret != (ssize_t) webm->output_position) {
        return self->error = SHOUTERR_SOCKET;
    }

    webm->output_position = 0;
    return self->error;
}

/* -- EBML helper functions -- */

/* Try to parse an EBML tag at the given location, returning the
 * length of the tag & the length of the associated payload.
 *
 * Returns the length of the tag on success, and writes the payload
 * size to *payload_length.
 *
 * Return 0 if it would be necessary to read past the
 * given end-of-buffer address to read a complete tag.
 *
 * Returns -1 if the tag is corrupt.
 */

static ssize_t ebml_parse_tag(unsigned char *buffer,
                              unsigned char *buffer_end,
                              uint64_t *tag_id,
                              uint64_t *payload_length)
{
    ssize_t type_length;
    ssize_t size_length;

    *tag_id = 0;
    *payload_length = 0;

    /* read past the type tag */
    type_length = ebml_parse_var_int(buffer, buffer_end, tag_id);

    if (type_length <= 0) {
        return type_length;
    }

    /* read the length tag */
    size_length = ebml_parse_var_int(buffer + type_length, buffer_end, payload_length);

    if (size_length <= 0) {
        return size_length;
    }

    return type_length + size_length;
}

/* Try to parse an EBML variable-length integer.
 * Returns 0 if there's not enough space to read the number;
 * Returns -1 if the number is malformed.
 * Else, returns the length of the number in bytes and writes the
 * value to *out_value.
 */
static ssize_t ebml_parse_var_int(unsigned char *buffer,
                                  unsigned char *buffer_end,
                                  uint64_t *out_value)
{
    ssize_t size = 1;
    ssize_t i;
    unsigned char mask = 0x80;
    uint64_t value;
    uint64_t unknown_marker;

    if (buffer >= buffer_end) {
        return 0;
    }

    /* find the length marker bit in the first byte */
    value = buffer[0];

    while (mask) {
        if (value & mask) {
            value = value & ~mask;
            unknown_marker = mask - 1;
            break;
        }
        size++;
        mask = mask >> 1;
    }

    /* catch malformed number (no prefix) */
    if (mask == 0) {
        return -1;
    }

    /* catch number bigger than parsing buffer */
    if (buffer + size - 1 >= buffer_end) {
        return 0;
    }

    /* read remaining bytes of (big-endian) number */
    for (i = 1; i < size; i++) {
        value = (value << 8) + buffer[i];
        unknown_marker = (unknown_marker << 8) + 0xFF;
    }

    /* catch special "unknown" length */

    if (value == unknown_marker) {
        *out_value = EBML_UNKNOWN;
    } else {
        *out_value = value;
    }

    return size;
}

/* Parse a big-endian int that may be from 1-8 bytes long.
 * Returns 0 if there's not enough space to read the number;
 * Returns -1 if the number is mis-sized.
 * Else, returns the length of the number in bytes and writes the
 * value to *out_value.
 * If is_signed is true, then the int is assumed to be two's complement
 * signed, negative values will be correctly promoted, and the returned
 * unsigned number can be safely cast to a signed number on systems using
 * two's complement arithmatic.
 */
static ssize_t ebml_parse_sized_int(unsigned char       *buffer,
                                    unsigned char       *buffer_end,
                                    size_t              len,
                                    bool                 is_signed,
                                    uint64_t  *out_value)
{
    uint64_t value;
    size_t i;

    if (len < 1 || len > 8) {
        return -1;
    }

    if (buffer + len >= buffer_end) {
        return 0;
    }

    if (is_signed && ((signed char) buffer[0]) < 0) {
        value = -1;
    } else {
        value = 0;
    }

    for (i = 0; i < len; i++) {
        value = (value << 8) + ((unsigned char) buffer[i]);
    }

    *out_value = value;

    return len;
}
