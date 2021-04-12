/* util.c: libshout utility/portability functions
 *
 *  Copyright 2002-2003 the Icecast team <team@icecast.org>,
 *  Copyright 2012-2015 Philipp "ph3-der-loewe" Schafft <lion@lion.leolix.org>
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

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef HAVE_SYS_SOCKET_H
#   include <sys/socket.h>
#endif

#ifdef HAVE_WINSOCK2_H
#   include <winsock2.h>
#endif

#include <shoutidjc/shout.h>

#include "util.h"

/* first all static tables, then the code */

static const char hexchars[16] = {
    '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'
};

/* For the next to tables see RFC3986 section 2.2 and 2.3. */
static const char safechars[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const char safechars_plus_gen_delims_minus_3F_and_23[256] = {
/* x0 x1 x2 x3 x4 x5 x6 x7 x8 x9 xa xb xc xd xe xf */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 1x */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, /* 2x */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, /* 3x */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 4x */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, /* 5x */
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 6x */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, /* 7x */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 8x */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 9x */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ax */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* bx */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* cx */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* dx */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* ex */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* fx */
};

static const char base64table[64] = {
    'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P',
    'Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f',
    'g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v',
    'w','x','y','z','0','1','2','3','4','5','6','7','8','9','+','/'
};


char *_shout_util_strdup(const char *s)
{
	if (!s)
        return NULL;

    return strdup(s);
}

int _shout_util_read_header(int sock, char *buff, unsigned long len)
{
    int read_bytes, ret;
    unsigned long pos;
    char c;

    read_bytes = 1;
    pos = 0;
    ret = 0;

    while ((read_bytes == 1) && (pos < (len - 1))) {
        read_bytes = 0;

        if ((read_bytes = recv(sock, &c, 1, 0))) {
            if (c != '\r')
                buff[pos++] = c;
            if ((pos > 1) && (buff[pos - 1] == '\n' &&
                              buff[pos - 2] == '\n')) {
                ret = 1;
                break;
            }
        } else {
            break;
        }
    }

	if (ret)
        buff[pos] = '\0';

    return ret;
}

/* This isn't efficient, but it doesn't need to be */
char *_shout_util_base64_encode(char *data)
{
    size_t len = strlen(data);
    char *out = malloc(len * 4 / 3 + 4);
    char *result = out;
    size_t chunk;

    while (len > 0) {
        chunk = (len > 3) ? 3 : len;
        *out++ = base64table[(*data & 0xFC) >> 2];
        *out++ = base64table[((*data & 0x03) << 4) | ((*(data + 1) & 0xF0) >> 4)];

        switch (chunk) {
            case 3:
                *out++ = base64table[((*(data + 1) & 0x0F) << 2) | ((*(data + 2) & 0xC0) >> 6)];
                *out++ = base64table[(*(data + 2)) & 0x3F];
            break;
            case 2:
                *out++ = base64table[((*(data + 1) & 0x0F) << 2)];
                *out++ = '=';
            break;
            case 1:
                *out++ = '=';
                *out++ = '=';
            break;
        }
        data += chunk;
        len -= chunk;
    }
    *out = 0;

    return result;
}

/* modified from libshout1, which credits Rick Franchuk <rickf@transpect.net>.
 * Caller must free result.
 */
static char *_url_encode_with_table(const char *data, const char table[256])
{
    const char *p;
    char *q, *dest;
    int digit;
    size_t n;

    for (p = data, n = 0; *p; p++) {
        n++;
        if (!table[(unsigned char)(*p)])
            n += 2;
    }

    if (!(dest = malloc(n+1))) return NULL;

    for (p = data, q = dest; *p; p++, q++) {
        if (table[(unsigned char)(*p)]) {
            *q = *p;
        } else {
            *q++ = '%';
            digit = (*p >> 4) & 0xF;
            *q++ = hexchars[digit];
            digit = *p & 0xf;
            *q = hexchars[digit];
            n += 2;
        }
    }
    *q = '\0';

    return dest;
}

char *_shout_util_url_encode(const char *data)
{
    return _url_encode_with_table(data, safechars);
}

char *_shout_util_url_encode_resource(const char *data)
{
    return _url_encode_with_table(data, safechars_plus_gen_delims_minus_3F_and_23);
}

util_dict *_shout_util_dict_new(void)
{
    return (util_dict*)calloc(1, sizeof(util_dict));
}

void _shout_util_dict_free(util_dict *dict)
{
    util_dict *next;

    while (dict) {
        next = dict->next;

		if (dict->key)
            free(dict->key);
		if (dict->val)
            free(dict->val);
        free(dict);

        dict = next;
    }
}

const char *_shout_util_dict_get(util_dict *dict, const char *key)
{
    while (dict) {
        if (dict->key && !strcmp(key, dict->key))
            return dict->val;
        dict = dict->next;
    }

    return NULL;
}

int _shout_util_dict_set(util_dict *dict, const char *key, const char *val)
{
    util_dict *prev;

    if (!dict || !key) {
        return SHOUTERR_INSANE;
    }

    prev = NULL;
    while (dict) {
        if (!dict->key || !strcmp(dict->key, key))
            break;
        prev = dict;
        dict = dict->next;
    }

    if (!dict) {
        dict = _shout_util_dict_new();
        if (!dict)
            return SHOUTERR_MALLOC;
        if (prev)
            prev->next = dict;
    }

    if (dict->key) {
        free(dict->val);
    } else if (!(dict->key = strdup(key))) {
        if (prev)
            prev->next = NULL;
        _shout_util_dict_free(dict);

        return SHOUTERR_MALLOC;
    }

    dict->val = strdup(val);
    if (!dict->val) {
        return SHOUTERR_MALLOC;
    }

    return SHOUTERR_SUCCESS;
}

/* given a dictionary, URL-encode each key and val and stringify them in order as
 * key=val&key=val... if val is set, or just key&key if val is NULL.
 *
 * TODO: Memory management needs overhaul.
 */
char *_shout_util_dict_urlencode(util_dict *dict, char delim)
{
    size_t reslen, resoffset;
    char *res, *tmp;
    char *enc;
    int start = 1;

    for (res = NULL; dict; dict = dict->next) {
        /* encode key */
        if (!dict->key)
            continue;
        if (!(enc = _shout_util_url_encode(dict->key))) {
            if (res)
                free(res);
            return NULL;
        }
        if (start) {
            reslen = strlen(enc) + 1;
            if (!(res = malloc(reslen))) {
                free(enc);
                return NULL;
            }
            snprintf(res, reslen, "%s", enc);
            free(enc);
            start = 0;
        } else {
            resoffset = strlen(res);
            reslen = resoffset + strlen(enc) + 2;
            if (!(tmp = realloc(res, reslen))) {
                free(enc);
                free(res);
                return NULL;
            } else {
                res = tmp;
            }
            snprintf(res + resoffset, reslen - resoffset, "%c%s", delim, enc);
            free(enc);
        }

        /* encode value */
        if (!dict->val)
            continue;
        if (!(enc = _shout_util_url_encode(dict->val))) {
            free(res);
            return NULL;
        }

        resoffset = strlen(res);
        reslen = resoffset + strlen(enc) + 2;
        if (!(tmp = realloc(res, reslen))) {
            free(enc);
            free(res);
            return NULL;
        } else {
            res = tmp;
        }
        snprintf(res + resoffset, reslen - resoffset, "=%s", enc);
        free(enc);
    }

    return res;
}

const char *_shout_util_dict_next(util_dict **dict, const char **key, const char **val)
{
    *key = NULL;
    *val = NULL;

    if (!dict)
        return NULL;
    *dict = (*dict)->next;
    while (*dict && !(*dict)->key)
        *dict = (*dict)->next;
    if (!*dict)
        return NULL;
    *key = (*dict)->key;
    *val = (*dict)->val;
    return *key;
}
