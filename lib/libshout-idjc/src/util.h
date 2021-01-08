/* util.h: libshout utility/portability functions
 *
 *  Copyright 2002-2004 the Icecast team <team@icecast.org>,
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
 *
 * $Id$
 */

#ifndef __LIBSHOUT_UTIL_H__
#define __LIBSHOUT_UTIL_H__

/* String dictionary type, without support for NULL keys, or multiple
 * instances of the same key
 */
typedef struct _util_dict {
    char *key;
    char *val;
    struct _util_dict *next;
} util_dict;

char 		*_shout_util_strdup(const char *s);

util_dict 	*_shout_util_dict_new(void);
void 		 _shout_util_dict_free(util_dict *dict);

/* dict, key must not be NULL. */
int 		 _shout_util_dict_set(util_dict *dict, const char *key, const char *val);
const char 	*_shout_util_dict_get(util_dict *dict, const char *key);
char 		*_shout_util_dict_urlencode(util_dict *dict, char delim);

const char *_shout_util_dict_next(util_dict **dict, const char **key, const char **val);

#define _SHOUT_DICT_FOREACH(init, var, keyvar, valvar) for ((var) = (init), (keyvar) = (var)->key ? (var)->key : _shout_util_dict_next(& (var), & (keyvar), & (valvar)), (valvar) = (var)->val; (var); _shout_util_dict_next(& (var), & (keyvar), & (valvar)))

char 	*_shout_util_base64_encode(char *data);
char 	*_shout_util_url_encode(const char *data);
char 	*_shout_util_url_encode_resource(const char *data);
int  	 _shout_util_read_header(int sock, char *buff, unsigned long len);

#endif /* __LIBSHOUT_UTIL_H__ */
