/* -*- c-basic-offset: 8; -*- */
/* proto_roaraudio.c: RoarAudio protocol support.
 * $Id$
 *
 *  Copyright (C) 2015      Philipp "ph3-der-loewe" Schafft <lion@lion.leolix.org>
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

#ifdef HAVE_INTTYPES_H
#   include <inttypes.h>
#endif

/* for htonl(). */
#ifdef HAVE_ARPA_INET_H
#   include <arpa/inet.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <shoutidjc/shout.h>
#include "shout_private.h"

typedef enum {
    STATE_IDENT = 0,
    STATE_AUTH,
    STATE_NEW_STREAM,
    STATE_EXEC
} shout_roar_protocol_state_t;

typedef enum {
    CMD_IDENTIFY    = 1,
    CMD_AUTH        = 2,
    CMD_NEW_STREAM  = 3,
    CMD_EXEC_STREAM = 5,
    CMD_OK          = 254
} shout_roar_command_t;

#define STREAM_NONE ((uint16_t)0xFFFF)
#define HEADER_SIZE 10

static int command_send(shout_t                *self,
                        shout_roar_command_t    command,
                        uint16_t                stream,
                        const void             *data,
                        size_t                  datalen)
{
    uint8_t header[HEADER_SIZE];

    if (!self)
        return SHOUTERR_INSANE;

    if (datalen > 65535)
        return SHOUTERR_INSANE;

    if (datalen && !data)
        return SHOUTERR_INSANE;

    /* version.
     * While version 2 is already on it's way we still go for version 0
     * as it will work well for us and is defined as part of the core
     * every RoarAudio server MUST implement.
     */
    header[0] = 0;
    /* command ID. */
    header[1] = command;
    /* stream ID. First upper then lower byte. */
    header[2] = (stream & 0xFF00) >> 8;
    header[3] = (stream & 0x00FF);
    /* now 4 bytes of stream position.
     * This implementation doesn't need this so we
     * set it to all zeros.
     */
    header[4] = 0;
    header[5] = 0;
    header[6] = 0;
    header[7] = 0;
    /* Now body ("data") size. First upper then lower byte. */
    header[8] = (datalen & 0xFF00) >> 8;
    header[9] = (datalen & 0x00FF);

    shout_queue_data(&self->wqueue, header, HEADER_SIZE);
    if (datalen)
        shout_queue_data(&self->wqueue, data, datalen);

    return SHOUTERR_SUCCESS;
}

static int shout_create_roaraudio_request_ident(shout_t *self)
{
    int         ret;
    size_t      datalen;
    uint8_t    *data;
    const char *agent;
    uint32_t    pid = getpid();

    /* We implement version 1 IDENTIFY header.
     * It has the following structure:
     * byte 0:     version (1).
     * byte 1-4:   PID in big endian.
     * byte 5-end: client name.
     */

    agent = shout_get_agent(self);
    if (!agent)
        return SHOUTERR_INSANE;

    datalen = 5 + strlen(agent);
    data = malloc(datalen);
    if (!data)
        return SHOUTERR_MALLOC;

    /* version number (1). */
    data[0] = 1;
    /* PID */
    data[1] = (pid & 0xFF000000UL) >> 24;
    data[2] = (pid & 0x00FF0000UL) >> 16;
    data[3] = (pid & 0x0000FF00UL) >> 8;
    data[4] = (pid & 0x000000FFUL) >> 0;
    /* agent name */
    memcpy(data + 5, agent, datalen - 5);

    ret = command_send(self, CMD_IDENTIFY, STREAM_NONE, data, datalen);

    free(data);

    return ret;
}

static int shout_create_roaraudio_request_auth(shout_t *self)
{
    /* Now we send an AUTH command to the server.
     * We currently only implement the NONE type.
     * NONE type is assumed by the server if
     * we send an empty body.
     */
    return command_send(self, CMD_AUTH, STREAM_NONE, NULL, 0);
}

static int shout_create_roaraudio_request_new_stream(shout_t *self)
{
    uint32_t data[6];

    /* We implement 24 byte NEW_STREAM structure (version 0).
     * It has the following structure:
     * byte  0- 3: stream direction [0].
     * byte  4- 7: Rel Pos ID (here: -1=NONE)
     * byte  8-11: Sample Rate[1].
     * byte 12-15: Bits per Sample[1].
     * byte 16-19: Number of Channels[1].
     * byte 20-23: Codec ID[2].
     *
     * The following asumptions are based on us only supporting
     * Ogg-based for now.
     * [0] = We currently only suport playback of waveform signals (1).
     *       See https://bts.keep-cool.org/wiki/Specs/DirValues
     * [1] = Server should detect automagically. defaulting to: 44100/16/2.
     * [2] = Ogg/Vorbis = 0x0010, Ogg/Speex = 0x0012, Ogg/FLAC = 0x0014,
     *       Ogg/CELT = 0x0016, Ogg/GENERAL (unknown logical streams) = 0x0015.
     *       See https://bts.keep-cool.org/wiki/Specs/CodecsValues
     */

    data[0] = htonl(1);
    data[1] = htonl((uint32_t)-1);
    data[2] = htonl(44100);
    data[3] = htonl(32);
    data[4] = htonl(2);
    data[5] = htonl(0x0010);  /* we assume Ogg/Vorbis for now. */

    return command_send(self, CMD_NEW_STREAM, STREAM_NONE, data, 24);
}

static int shout_create_roaraudio_request_exec(shout_t *self)
{
    /* Last an EXEC_STREAM command should be sent to open
     * an IO channel for the new stream.
     * If successful the control socket will be used for data
     * after that. This very much like with SOURCE requests.
     * so no hard deal to intigrate.
     */
    return command_send(self, CMD_EXEC_STREAM, self->protocol_extra, NULL, 0);
}

int shout_create_roaraudio_request(shout_t *self)
{
    switch ((shout_roar_protocol_state_t)self->protocol_state) {
    case STATE_IDENT:
        return shout_create_roaraudio_request_ident(self);
        break;
    case STATE_AUTH:
        return shout_create_roaraudio_request_auth(self);
        break;
    case STATE_NEW_STREAM:
        return shout_create_roaraudio_request_new_stream(self);
        break;
    case STATE_EXEC:
        return shout_create_roaraudio_request_exec(self);
        break;
    }

    return SHOUTERR_INSANE;
}

int shout_get_roaraudio_response(shout_t *self)
{
    shout_buf_t   *queue;
    size_t         total_len = 0;
    uint8_t        header[HEADER_SIZE];

    for (queue = self->rqueue.head; queue; queue = queue->next) {
        if (total_len < 10)
            memcpy(header + total_len, queue->data, queue->len > (HEADER_SIZE - total_len) ? (HEADER_SIZE - total_len) : queue->len);
        total_len += queue->len;
    }

    /* the header alone has 10 bytes. */
    if (total_len < HEADER_SIZE)
        return SHOUTERR_BUSY;

    /* ok. we got a header.
     * Now find the body length ("data length") bytes
     * and see if they are both zero.
     * If not the server sent us extra infos we currently
     * not support.
     */

    if (header[8] || header[9])
        return SHOUTERR_UNSUPPORTED;

    /* Hey, we got a response. */
    return SHOUTERR_SUCCESS;
}

int shout_parse_roaraudio_response(shout_t *self)
{
    char *data = NULL;
    uint8_t header[HEADER_SIZE];

    /* ok, this is the most hacky function in here as we do not
     * use a well designed and universal parser for the responses.
     * Yet there is little need for it.
     * We just need to check if we got an CMD_OK and
     * pull out the stream ID in case of STATE_NEW_STREAM.
     * "data length" is already checked by shout_get_roaraudio_response().
     */

    if (shout_queue_collect(self->rqueue.head, &data) != HEADER_SIZE) {
        free(data);
        return SHOUTERR_INSANE;
    }
    shout_queue_free(&self->rqueue);
    memcpy(header, data, HEADER_SIZE);
    free(data);

    /* check version */
    if (header[0] != 0)
        return SHOUTERR_UNSUPPORTED;

    /* have we got a positive response? */
    if (header[1] != CMD_OK)
        return SHOUTERR_NOLOGIN;

    switch ((shout_roar_protocol_state_t)self->protocol_state) {
        case STATE_IDENT:
            self->protocol_state = STATE_AUTH;
        break;

        case STATE_AUTH:
            self->protocol_state = STATE_NEW_STREAM;
        break;

        case STATE_NEW_STREAM:
            self->protocol_extra = (((unsigned int)header[2]) << 8) | (unsigned int)header[3];
            self->protocol_state = STATE_EXEC;
        break;

        case STATE_EXEC:
            /* ok. everything worked. Continue normally! */
            return SHOUTERR_SUCCESS;
        break;

        default:
            return SHOUTERR_INSANE;
        break;
    }

    self->state = SHOUT_STATE_REQ_CREATION;
    return SHOUTERR_RETRY;
}
