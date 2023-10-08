/* legacy.c: Implementation of public libshout interface shout.h
 *           legacy/obsolete functions only.
 *
 *  Copyright (C) 2002-2004 the Icecast team <team@icecast.org>,
 *  Copyright (C) 2012-2022 Philipp Schafft <lion@lion.leolix.org>
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
#include <config.h>
#endif

#include <shoutidjc/shout.h>

#include "shout_private.h"
#include "util.h"

int shout_set_format(shout_t *self, unsigned int format)
{
    if (!self)
        return SHOUTERR_INSANE;

    if (self->connection)
        return self->error = SHOUTERR_CONNECTED;

    switch (format) {
        case SHOUT_FORMAT_OGG:
            return shout_set_content_format(self, SHOUT_FORMAT_OGG, SHOUT_USAGE_UNKNOWN, NULL);
        break;
        case SHOUT_FORMAT_MP3:
            return shout_set_content_format(self, SHOUT_FORMAT_MP3, SHOUT_USAGE_AUDIO, NULL);
        break;
        case SHOUT_FORMAT_WEBM:
            return shout_set_content_format(self, SHOUT_FORMAT_WEBM, SHOUT_USAGE_AUDIO|SHOUT_USAGE_VISUAL, NULL);
        break;
        case SHOUT_FORMAT_WEBMAUDIO:
            return shout_set_content_format(self, SHOUT_FORMAT_WEBM, SHOUT_USAGE_AUDIO, NULL);
        break;
    }

    return self->error = SHOUTERR_UNSUPPORTED;
}

unsigned int shout_get_format(shout_t* self)
{
    if (!self)
        return 0;

    if (self->format == SHOUT_FORMAT_WEBM && self->usage == SHOUT_USAGE_AUDIO) {
        return SHOUT_FORMAT_WEBMAUDIO;
    }

    return self->format;
}

int shout_set_dumpfile(shout_t *self, const char *dumpfile)
{
    if (!self)
        return SHOUTERR_INSANE;

    if (self->connection)
        return SHOUTERR_CONNECTED;

    if (self->dumpfile)
        free(self->dumpfile);

    if ( !(self->dumpfile = _shout_util_strdup(dumpfile)) )
        return self->error = SHOUTERR_MALLOC;

    return self->error = SHOUTERR_SUCCESS;
}

const char *shout_get_dumpfile(shout_t *self)
{
    if (!self)
        return NULL;

    return self->dumpfile;
}

int shout_set_name(shout_t *self, const char *name)
{
    return shout_set_meta(self, "name", name);
}

const char *shout_get_name(shout_t *self)
{
    return shout_get_meta(self, "name");
}

int shout_set_url(shout_t *self, const char *url)
{
    return shout_set_meta(self, "url", url);
}

const char *shout_get_url(shout_t *self)
{
    return shout_get_meta(self, "url");
}

int shout_set_genre(shout_t *self, const char *genre)
{
    return shout_set_meta(self, "genre", genre);
}

const char *shout_get_genre(shout_t *self)
{
    return shout_get_meta(self, "genre");
}

int shout_set_description(shout_t *self, const char *description)
{
    return shout_set_meta(self, "description", description);
}

const char *shout_get_description(shout_t *self)
{
    return shout_get_meta(self, "description");
}

