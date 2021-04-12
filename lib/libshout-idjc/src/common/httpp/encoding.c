/* encoding.c
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

#ifdef HAVE_CONFIG_H
 #include <config.h>
#endif

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "encoding.h"

struct httpp_encoding_tag {
    size_t refc;

    ssize_t (*process_read)(httpp_encoding_t *self, void *buf, size_t len, ssize_t (*cb)(void*, void*, size_t), void *userdata);
    ssize_t (*process_write)(httpp_encoding_t *self, const void *buf, size_t len, ssize_t (*cb)(void*, const void*, size_t), void *userdata);

    httpp_meta_t *meta_read;
    httpp_meta_t *meta_write;

    void *buf_read_raw; /* input buffer */
    size_t buf_read_raw_offset, buf_read_raw_len;

    void *buf_read_decoded; /* decoded stuff */
    size_t buf_read_decoded_offset, buf_read_decoded_len;

    void *buf_write_raw; /* input buffer */
    size_t buf_write_raw_offset, buf_write_raw_len;

    void *buf_write_encoded; /* encoded output */
    size_t buf_write_encoded_offset, buf_write_encoded_len;

    /* backend specific stuff */
    size_t read_bytes_till_header;
};


/* Handlers, at end of file */
static ssize_t __enc_identity_read(httpp_encoding_t *self, void *buf, size_t len, ssize_t (*cb)(void*, void*, size_t), void *userdata);
static ssize_t __enc_identity_write(httpp_encoding_t *self, const void *buf, size_t len, ssize_t (*cb)(void*, const void*, size_t), void *userdata);
static ssize_t __enc_chunked_read(httpp_encoding_t *self, void *buf, size_t len, ssize_t (*cb)(void*, void*, size_t), void *userdata);
static ssize_t __enc_chunked_write(httpp_encoding_t *self, const void *buf, size_t len, ssize_t (*cb)(void*, const void*, size_t), void *userdata);

/* function to move some data out of our buffers */
ssize_t __copy_buffer(void *dst, void **src, size_t *boffset, size_t *blen, size_t len)
{
    char *p;
    size_t have_len;
    size_t todo;

    if (!len)
        return 0;

    if (!dst || !src || !*src || !boffset || !blen)
        return -1;

    have_len = *blen - *boffset;
    p = (char *)*src + *boffset;

    todo = len < have_len ? len : have_len;

    memcpy(dst, p, todo);

    *boffset += todo;

    if (*boffset == *blen) {
        free(*src);
        *src = NULL;
        *boffset = 0;
        *blen = 0;
    }

    return todo;
}

/* try to flush output buffers */
static inline void __flush_output(httpp_encoding_t *self, ssize_t (*cb)(void*, const void*, size_t), void *userdata)
{
    if (cb && self->buf_write_encoded) {
        ssize_t ret = cb(userdata,
                         (char *)self->buf_write_encoded + self->buf_write_encoded_offset,
                         self->buf_write_encoded_len - self->buf_write_encoded_offset);
        if (ret > 0) {
            self->buf_write_encoded_offset += ret;
            if (self->buf_write_encoded_offset == self->buf_write_encoded_len) {
                free(self->buf_write_encoded);
                self->buf_write_encoded = NULL;
                self->buf_write_encoded_offset = 0;
                self->buf_write_encoded_len = 0;
            }
        }
    }
}

/* meta data functions */
/* meta data is to be used in a encoding-specific way */
httpp_meta_t     *httpp_encoding_meta_new(const char *key, const char *value)
{
    httpp_meta_t *ret = calloc(1, sizeof(httpp_meta_t));
    if (!ret)
        return ret;

    if (key)
       if ((ret->key = strdup(key)) == NULL)
           goto fail;

    if (value) {
       if ((ret->value = strdup(value)) == NULL)
           goto fail;
       ret->value_len = strlen(ret->value);
    }

    return ret;
fail:
    httpp_encoding_meta_free(ret);
    return NULL;
}

int               httpp_encoding_meta_free(httpp_meta_t *self)
{
    while (self) {
        httpp_meta_t *cur = self;
        self = self->next;

        if (cur->key)
            free(cur->key);
        if (cur->value)
            free(cur->value);
        free(cur);
    }

    return 0;
}

int               httpp_encoding_meta_append(httpp_meta_t **dst, httpp_meta_t *next)
{
    httpp_meta_t **cur;

    if (!dst)
        return -1;
    if (!next)
        return 0;

    cur = dst;
    while (*cur)
        cur = &((*cur)->next);

    *cur = next;

    return 0; 
}

/* General setup */
httpp_encoding_t *httpp_encoding_new(const char *encoding) {
    httpp_encoding_t *ret = calloc(1, sizeof(httpp_encoding_t));
    if (!ret)
        return NULL;

    ret->refc = 1;

    if (strcasecmp(encoding, HTTPP_ENCODING_IDENTITY) == 0) {
        ret->process_read = __enc_identity_read;
        ret->process_write = __enc_identity_write;
    } else if (strcasecmp(encoding, HTTPP_ENCODING_CHUNKED) == 0) {
        ret->process_read = __enc_chunked_read;
        ret->process_write = __enc_chunked_write;
    } else {
        goto fail;
    }

    return ret;

fail:
    httpp_encoding_release(ret);
    return NULL;
}

int               httpp_encoding_addref(httpp_encoding_t *self)
{
    if (!self)
        return -1;
    self->refc++;
    return 0;
}

int               httpp_encoding_release(httpp_encoding_t *self)
{
    if (!self)
        return -1;

    self->refc--;
    if (self->refc)
        return 0;

    httpp_encoding_meta_free(self->meta_read);
    httpp_encoding_meta_free(self->meta_write);

    if (self->buf_read_raw)
        free(self->buf_read_raw);
    if (self->buf_read_decoded)
        free(self->buf_read_decoded);
    if (self->buf_write_raw)
        free(self->buf_write_raw);
    if (self->buf_write_encoded)
        free(self->buf_write_encoded);
    free(self);
    return 0;
}

/* Read data from backend.
 * if cb is NULL this will read from the internal buffer.
 */
ssize_t           httpp_encoding_read(httpp_encoding_t *self, void *vbuf, size_t len, ssize_t (*cb)(void*, void*, size_t), void *userdata)
{
    ssize_t done = 0;
    ssize_t ret;
    char *buf = vbuf;

    if (!self || !buf)
        return -1;

    if (!len)
        return 0;

    ret = __copy_buffer(buf, &(self->buf_read_decoded), &(self->buf_read_decoded_offset), &(self->buf_read_decoded_len), len);

    if (ret == (ssize_t)len)
        return ret;

    if (ret > 0) {
        done += ret;
        buf  += ret;
        len  -= ret;
    }

    ret = self->process_read(self, buf, len, cb, userdata);
    if (ret == -1 && done)
        return done;
    if (ret == -1)
        return -1;

    done += ret;
    buf  += ret;
    len  -= ret;

    if (len) {
        ret = __copy_buffer(buf, &(self->buf_read_decoded), &(self->buf_read_decoded_offset), &(self->buf_read_decoded_len), len);
        if (ret > 0) {
            done += ret;
            buf  += ret;
            len  -= ret;
        }
    }

    return done;
}

/* Read any meta data that is in buffer.
 * After a call to this function the meta data is released from the
 * encoding object and the caller is responsible to free it.
 */
httpp_meta_t     *httpp_encoding_get_meta(httpp_encoding_t *self)
{
    httpp_meta_t *ret;

    if (!self)
        return NULL;

    ret = self->meta_read;
    self->meta_read = NULL;
    return ret;
}

/* Write data to backend.
 * If buf is NULL this will flush buffers.
 * Depending on encoding flushing buffers may not be safe if not
 * at end of stream.
 */
ssize_t           httpp_encoding_write(httpp_encoding_t *self, const void *buf, size_t len, ssize_t (*cb)(void*, const void*, size_t), void *userdata)
{
    ssize_t ret;

    if (!self || !cb)
        return -1;

    /* first try to flush buffers */
    __flush_output(self, cb, userdata);

    /* now run the processor */
    ret = self->process_write(self, buf, len, cb, userdata);

    /* try to flush buffers again, maybe they are filled now! */
    __flush_output(self, cb, userdata);

    return ret;
}

/* Check if we have something to flush. */
ssize_t           httpp_encoding_pending(httpp_encoding_t *self)
{
    if (!self)
        return -1;
    if (!self->buf_write_encoded)
        return 0;
    return self->buf_write_encoded_len - self->buf_write_encoded_offset;
}

/* Attach meta data to the stream.
 * this is to be written out as soon as the encoding supports.
 */
int               httpp_encoding_append_meta(httpp_encoding_t *self, httpp_meta_t *meta)
{
    if (!self)
        return -1;
    return httpp_encoding_meta_append(&(self->meta_write), meta);
}

/* handlers for encodings */
static ssize_t __enc_identity_read(httpp_encoding_t *self, void *buf, size_t len, ssize_t (*cb)(void*, void*, size_t), void *userdata)
{
    (void)self;
    if (!cb)
        return -1;
    return cb(userdata, buf, len);
}

static ssize_t __enc_identity_write(httpp_encoding_t *self, const void *buf, size_t len, ssize_t (*cb)(void*, const void*, size_t), void *userdata)
{
    (void)self;
    if (!cb)
        return -1;
    return cb(userdata, buf, len);
}

/* Here is what chunked encoding looks like:
 *
 * You have any number of chunks. They are just chained.
 * The last chunk has a length of zero set.
 * After the last chunk there are tailing headers and a final "\r\n".
 *
 * Each chunk looks like this:
 * LENGTH as hex [";" extension [";" extention [...]]] "\r\n" BODY "\r\n".
 * extension is: TOKEN ["=" data];
 * data is: TOKEN | STRING as quoted.
 *
 */

static void __enc_chunked_read_extentions(httpp_encoding_t *self, char *p, size_t len)
{
    /* ok. If you want to ruin your day... go ahead and try to understand this. */
    httpp_meta_t **parent;
    httpp_meta_t *meta;
    size_t value_len;
    size_t value_decoded_len;
    size_t key_len;
    char *value;
    char *c;
    int in_quote;

    if (self->meta_read) {
        parent = &(self->meta_read);
        while (*parent)
            parent = &((*parent)->next);
    } else {
        parent = &(self->meta_read);
    }

    while (len) {
        if (*p != ';') /* not a valid extension */
            break;
        p++;
        len--;

        *parent = meta = httpp_encoding_meta_new(NULL, NULL);
        parent = &(meta->next);

        for (key_len = 0; key_len < len && p[key_len] != '='; key_len++);
        meta->key = malloc(key_len + 1);
        if (!meta->key)
            return;
        memcpy(meta->key, p, key_len);
        meta->key[key_len] = 0;
        p += key_len;
        len -= key_len;
        if (!len)
            break;

        if (*p != '=') /* check if we have a value */
            continue;

        p++;
        len--;

        if (!len)
            break;

        in_quote = 0;
        for (value_len = 0; value_len < len; value_len++) {
            if (in_quote) {
                if (p[value_len] == '\\') in_quote = 2;
                else if (p[value_len] == '"') in_quote--;
                if (in_quote)
                    continue;
                value_len++;
                break;
            }
            if (p[value_len] == '"') {
                in_quote = 1;
            } else if (p[value_len] == ';')
                break;
        }

        value = meta->value = malloc(value_len + 1);
        if (!meta->value)
            return;
        in_quote = 0;
        for (c = p, p += value_len, len -= value_len, value_decoded_len = 0; value_len; value_len--, c++) {
            if (in_quote) {
                if (*c == '\\') in_quote = 2;
                else if (*c == '"') {
                    in_quote--;
                    if (in_quote)
                        value[value_decoded_len++] = '"';
                } else
                    value[value_decoded_len++] = *c;
                if (in_quote)
                    continue;
                break;
            }
            if (*c == '"') {
                in_quote = 1;
            } else if (p[value_len] == ';')
                break;
            else
                value[value_decoded_len++] = *c;
        }
        value[value_decoded_len] = 0;
        meta->value_len = value_decoded_len;
    }
}

static ssize_t __enc_chunked_read(httpp_encoding_t *self, void *buf, size_t len, ssize_t (*cb)(void*, void*, size_t), void *userdata)
{
    ssize_t ret;
    size_t buflen;
    void *bufptr;
    size_t i;
    char *c;
    int in_quote;
    ssize_t offset_extentions;
    ssize_t offset_CR;
    ssize_t offset_LF;
    long long unsigned int bodylen;

    if (!cb)
        return -1;

    /* see if we have still a few bytes to go till the next header
     * The 2 is the end of chunk mark that is not part of the body! */
    if (self->read_bytes_till_header > 2) {
        size_t todo = len > (self->read_bytes_till_header - 2) ? (self->read_bytes_till_header - 2) : len;
        ret = cb(userdata, buf, todo);
        if (ret < 1)
            return ret;
        self->read_bytes_till_header -= ret;
        return ret;
    }

    /* ok. now we should have 2 or less bytes till next header.
     * We just read a few bytes into our decoding buffer and see what we got.
     */
    buflen = 1024;

    if (self->buf_read_raw) {
        bufptr = realloc(self->buf_read_raw, self->buf_read_raw_len + buflen);
    } else {
        bufptr = malloc(buflen);
        self->buf_read_raw_offset = 0;
        self->buf_read_raw_len = 0;
    }
    if (!bufptr)
        return -1;

    self->buf_read_raw = bufptr;

    ret = cb(userdata, (char *)self->buf_read_raw + self->buf_read_raw_len, buflen);
    if (ret < 1) {
        if (!self->buf_read_raw_len) {
            free(self->buf_read_raw);
            self->buf_read_raw = NULL;
        }
        return ret;
    }

    self->buf_read_raw_len += ret;

    /* now we should have some bytes in our buffer.
     * Now skip the "\r\n" that may be left to do from self->read_bytes_till_header.
     */

    if ((self->buf_read_raw_len - self->buf_read_raw_offset) == self->read_bytes_till_header) {
        /* ok, that wasn't super big step forward. At least we got rid of self->read_bytes_till_header.
         * now freeing buffers again...
         */
        free(self->buf_read_raw);
        self->buf_read_raw = NULL;
        self->buf_read_raw_offset = 0;
        self->buf_read_raw_len = 0;
        self->read_bytes_till_header = 0;
        return 0;
    } else if ((self->buf_read_raw_len - self->buf_read_raw_offset) > self->read_bytes_till_header) {
        /* ship the tailing "\r\n".
         * We should check we really got that and not some other stuff, eh? Yes, I'm sure we should...
         */
        self->buf_read_raw_offset += self->read_bytes_till_header;
        self->read_bytes_till_header = 0;
    }

    /* ok. next we have at least a little bit if a header.
     * Now we need to find out of the header is complet.
     * If it is we will process it. If it isn't we will
     * just return a short read and try again later!
     *
     * Hint: A complet header ends with "\r\n".
     * But be aware! That is valid in any quoted string
     * that is part of the header as well!
     */

    in_quote = 0;
    offset_extentions = -1;
    offset_CR = -1;
    offset_LF = -1;
    for (i = self->buf_read_raw_offset, c = (char *)self->buf_read_raw + self->buf_read_raw_offset;
         i < self->buf_read_raw_len;
         i++, c++) {
        if (in_quote) {
            if (*c == '\\') in_quote = 2;
            else if (*c == '"') in_quote--;
            continue;
        }
        if (*c == '"') {
            in_quote = 1;
        } else if (*c == ';' && offset_extentions == -1) {
            offset_extentions = i;
        } else if (*c == '\r') {
            offset_CR = i;
        } else if (*c == '\n' && offset_CR == (ssize_t)(i - 1)) {
            offset_LF = i;
            break;
        }
    }

    /* ok, now we know a lot more!:
     * offset_extentions is the offset to the extentions if any.
     * offset_LF is the offset to the byte before the body if any.
     * if offset_LF is > -1 we know we have a complet header.
     * if we don't just return a short read and try again later.
     */

    if (offset_LF == -1)
        return 0;

    /* ok. Now we have a complet header.
     * First pass the extentions to extention parser if any.
     */
    if (offset_extentions != -1)
        __enc_chunked_read_extentions(self, (char *)self->buf_read_raw + offset_extentions, offset_CR - offset_extentions);

    /* ok. Next we parse the body length.
     * We just replace whatever comes after the length by \0
     * and try to parse the hex value.
     */
    if (offset_extentions != -1) {
        c = (char *)self->buf_read_raw + offset_extentions;
    } else {
        c = (char *)self->buf_read_raw + offset_CR;
    }
    *c = 0;

    /* we hope that will work... */
    if (sscanf((char *)self->buf_read_raw + self->buf_read_raw_offset, "%llx", &bodylen) != 1)
        return -1;

    /* ok, Now we move the offset forward to the body. */
    self->buf_read_raw_offset = offset_LF + 1;

    /* Do we still have some data in buffer?
     * If not free the buffer and set the counters
     * to point to the next header.
     */
    if (self->buf_read_raw_offset == self->buf_read_raw_len) {
        free(self->buf_read_raw);
        self->buf_read_raw = NULL;
        self->buf_read_raw_offset = 0;
        self->buf_read_raw_len = 0;
        self->read_bytes_till_header = bodylen + 2; /* 2 = tailing "\r\n" */
        return 0;
    }

    /* ok, now we check if what we have in the buffer is less or equal than our bodylen. */
    if ((self->buf_read_raw_len - self->buf_read_raw_offset) <= bodylen) {
        /* ok, this is fantastic. The framework can do the rest for us! */
        self->buf_read_decoded = self->buf_read_raw;
        self->buf_read_decoded_offset = self->buf_read_raw_offset;
        self->buf_read_decoded_len = self->buf_read_raw_len;
        self->buf_read_raw = NULL;
        self->buf_read_raw_offset = 0;
        self->buf_read_raw_len = 0;
        self->read_bytes_till_header = bodylen + 2 - (self->buf_read_decoded_len - self->buf_read_decoded_offset);
        return 0;
    }

    /* ok, final case left:
     * we have more than just the body in our buffer.
     * What we do now is to allocate buffer, move the body over
     * and let our internal structures point to the next header.
     */

    self->buf_read_decoded = malloc(bodylen);
    if (!self->buf_read_decoded) /* just retry later if we can not allocate a buffer */
        return -1;
    self->buf_read_decoded_offset = 0;
    self->buf_read_decoded_len = bodylen;
    memcpy(self->buf_read_decoded, (char *)self->buf_read_raw + self->buf_read_raw_offset, bodylen);
    self->buf_read_raw_offset += bodylen;
    self->read_bytes_till_header = 2; /* tailing "\r\n" */

    return 0;
}

static size_t __enc_chunked_write_extensions_valuelen(httpp_meta_t *cur)
{
    size_t ret = cur->value_len;
    size_t i;
    char *p = cur->value;

    if (!cur->value || !cur->value_len)
        return 0;

    for (i = 0; i < cur->value_len; i++, p++)
        if (*p == '"')
            ret++;

    return ret;
}

static char *__enc_chunked_write_extensions(httpp_encoding_t *self)
{
    size_t buflen;
    void *buf;
    char *p;
    size_t len;
    httpp_meta_t *cur;

    if (!self->meta_write)
        return NULL;

    /* first find out how long the buffer must be. */
    buflen = 1; /* tailing \0 */

    cur = self->meta_write;
    while (cur) {
        if (!cur->key || (cur->value_len && !cur->value)) {
            cur = cur->next;
            continue;
        }

        buflen += 4; /* ; and = and two " */
        buflen += strlen(cur->key);
        buflen += __enc_chunked_write_extensions_valuelen(cur);
        cur = cur->next;
    }

    p = buf = malloc(buflen);
    if (!buf)
        return NULL;

    cur = self->meta_write;
    while (cur) {
        if (!cur->key || (cur->value_len && !cur->value)) {
            cur = cur->next;
            continue;
        }

        *(p++) = ';';
        len = strlen(cur->key);
        memcpy(p, cur->key, len);
        p += len;

        if (cur->value_len) {
            const char *c;
            size_t i;

            *(p++) = '=';
            *(p++) = '"';
            for (i = 0, c = cur->value; i < cur->value_len; i++, c++) {
                if (*c == '"')
                    *(p++) = '\\';
                *(p++) = *c;
            }
            *(p++) = '"';
        }

        cur = cur->next;
    }

    *p = 0; /* terminate the string */

    httpp_encoding_meta_free(self->meta_write);
    self->meta_write = NULL;

    return buf;
}
static ssize_t __enc_chunked_write(httpp_encoding_t *self, const void *buf, size_t len, ssize_t (*cb)(void*, const void*, size_t), void *userdata)
{
    char encoded_length[32];
    char *extensions = NULL;
    ssize_t total_chunk_size;
    ssize_t header_length;

    (void)cb, (void)userdata;

    if (!buf)
        len = 0;

    /* refuse to write if we still have stuff to flush. */
    if (httpp_encoding_pending(self) > 0)
        return 0;

    /* limit length to a bit more sane value */
    if (len > 1048576)
        len = 1048576;

    snprintf(encoded_length, sizeof(encoded_length), "%lx", (long int)len);

    extensions = __enc_chunked_write_extensions(self);

    /* 2 = end of header and tailing "\r\n" */
    header_length = strlen(encoded_length) + (extensions ? strlen(extensions) : 0) + 2;
    total_chunk_size = header_length + len + 2;
    if (!buf)
        total_chunk_size += 2;

    /* ok, we now allocate a huge buffer. We do it as if we would do it only when needed
     * and it would fail we would end in bad state that can not be recovered */
    self->buf_write_encoded = malloc(total_chunk_size);
    if (!self->buf_write_encoded) {
        if (extensions)
            free(extensions);
        return -1;
    }

    self->buf_write_encoded_offset = 0;
    self->buf_write_encoded_len = total_chunk_size;
    snprintf(self->buf_write_encoded, total_chunk_size, "%s%s\r\n", encoded_length, extensions ? extensions : "");
    memcpy((char *)self->buf_write_encoded + header_length, buf, len);
    memcpy((char *)self->buf_write_encoded + header_length + len, "\r\n\r\n", buf ? 2 : 4);

    if (extensions)
        free(extensions);

    return len;
}
