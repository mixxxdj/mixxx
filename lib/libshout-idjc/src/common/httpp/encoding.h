/* encoding.h
**
** http transfer encoding library
** See RFC2616 section 3.6 for more details.
**
** Copyright (C) 2015 Philipp "ph3-der-loewe" Schafft <lion@lion.leolix.org>
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the
** Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
** Boston, MA  02110-1301, USA.
**
*/

#ifndef __ENCODING_H
#define __ENCODING_H

#include <sys/types.h>

#if _WIN32
#include <os.h>
#endif

/* known encodings */
#define HTTPP_ENCODING_IDENTITY "identity" /* RFC2616 */
#define HTTPP_ENCODING_CHUNKED  "chunked"  /* RFC2616 */
#define HTTPP_ENCODING_GZIP     "gzip"     /* RFC1952 */
#define HTTPP_ENCODING_COMPRESS "compress" /* ??? */
#define HTTPP_ENCODING_DEFLATE  "deflate"  /* RFC1950, RFC1951 */

typedef struct httpp_encoding_tag httpp_encoding_t;

typedef struct httpp_meta_tag httpp_meta_t;
struct httpp_meta_tag {
    char *key;
    void *value;
    size_t value_len;
    httpp_meta_t *next;
};

/* meta data functions */
/* meta data is to be used in a encoding-specific way */
httpp_meta_t     *httpp_encoding_meta_new(const char *key, const char *value);
int               httpp_encoding_meta_free(httpp_meta_t *self);
int               httpp_encoding_meta_append(httpp_meta_t **dst, httpp_meta_t *next);

/* General setup */
httpp_encoding_t *httpp_encoding_new(const char *encoding);
int               httpp_encoding_addref(httpp_encoding_t *self);
int               httpp_encoding_release(httpp_encoding_t *self);

/* Read data from backend.
 * if cb is NULL this will read from the internal buffer.
 */
ssize_t           httpp_encoding_read(httpp_encoding_t *self, void *buf, size_t len, ssize_t (*cb)(void*, void*, size_t), void *userdata);

/* Read any meta data that is in buffer.
 * After a call to this function the meta data is released from the
 * encoding object and the caller is responsible to free it.
 */
httpp_meta_t     *httpp_encoding_get_meta(httpp_encoding_t *self);

/* Write data to backend.
 * If buf is NULL this will flush buffers.
 * Depending on encoding flushing buffers may not be safe if not
 * at end of stream.
 */
ssize_t           httpp_encoding_write(httpp_encoding_t *self, const void *buf, size_t len, ssize_t (*cb)(void*, const void*, size_t), void *userdata);

/* Check if we have something to flush. */
ssize_t           httpp_encoding_pending(httpp_encoding_t *self);

/* Attach meta data to the stream.
 * this is to be written out as soon as the encoding supports.
 */
int               httpp_encoding_append_meta(httpp_encoding_t *self, httpp_meta_t *meta);

#endif
