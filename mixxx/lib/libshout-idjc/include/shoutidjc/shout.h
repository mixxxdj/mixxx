/*  shout.h
 *
 *  API for libshout, the streaming library for icecast
 *
 *  Copyright (C) 2002-2003 the Icecast team <team@icecast.org>,
 *  Copyright (C) 2012-2015 Philipp "ph3-der-loewe" Schafft <lion@lion.leolix.org>
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
#ifndef __LIBSHOUT_SHOUT_H__
#define __LIBSHOUT_SHOUT_H__

#include <sys/types.h>
#if defined(WIN32) && !defined(__MINGW64__) && !defined(__MINGW32__)
#include <os.h>
#endif

#define SHOUTERR_SUCCESS            (  0) /* No error */
#define SHOUTERR_INSANE             ( -1) /* Nonsensical arguments e.g. self being NULL */ 
#define SHOUTERR_NOCONNECT          ( -2) /* Couldn't connect */
#define SHOUTERR_NOLOGIN            ( -3) /* Login failed */
#define SHOUTERR_SOCKET             ( -4) /* Socket error */
#define SHOUTERR_MALLOC             ( -5) /* Out of memory */
#define SHOUTERR_METADATA           ( -6)
#define SHOUTERR_CONNECTED          ( -7) /* Cannot set parameter while connected */
#define SHOUTERR_UNCONNECTED        ( -8) /* Not connected */
#define SHOUTERR_UNSUPPORTED        ( -9) /* This libshout doesn't support the requested option */
#define SHOUTERR_BUSY               (-10) /* Socket is busy */
#define SHOUTERR_NOTLS              (-11) /* TLS requested but not supported by peer */
#define SHOUTERR_TLSBADCERT         (-12) /* TLS connection can not be established because of bad certificate */
#define SHOUTERR_RETRY              (-13) /* Retry last operation. */

#define SHOUT_FORMAT_OGG            (  0) /* application/ogg */
#define SHOUT_FORMAT_MP3            (  1) /* audio/mpeg */
#define SHOUT_FORMAT_WEBM           (  2) /* video/webm */
#define SHOUT_FORMAT_WEBMAUDIO      (  3) /* audio/webm audio only */
#define SHOUT_FORMAT_AAC            ( 10) /* audio/aac */
#define SHOUT_FORMAT_AACPLUS        ( 11) /* audio/aacp */

/* backward-compatibility alias */
#define SHOUT_FORMAT_VORBIS         SHOUT_FORMAT_OGG

#define SHOUT_PROTOCOL_HTTP         (  0)
#define SHOUT_PROTOCOL_XAUDIOCAST   (  1)
#define SHOUT_PROTOCOL_ICY          (  2)
#define SHOUT_PROTOCOL_ROARAUDIO    (  3)

/* Possible TLS modes */
#define SHOUT_TLS_DISABLED          (  0) /* Do not use TLS at all */
#define SHOUT_TLS_AUTO              (  1) /* Autodetect which TLS mode to use if any */
#define SHOUT_TLS_AUTO_NO_PLAIN     (  2) /* Like SHOUT_TLS_AUTO_NO_PLAIN but does not allow plain connections */
#define SHOUT_TLS_RFC2818           ( 11) /* Use TLS for transport layer like HTTPS [RFC2818] does. */
#define SHOUT_TLS_RFC2817           ( 12) /* Use TLS via HTTP Upgrade:-header [RFC2817]. */

#define SHOUT_AI_BITRATE            "bitrate"
#define SHOUT_AI_SAMPLERATE         "samplerate"
#define SHOUT_AI_CHANNELS           "channels"
#define SHOUT_AI_QUALITY            "quality"

#define SHOUT_META_NAME             "name"
#define SHOUT_META_URL              "url"
#define SHOUT_META_GENRE            "genre"
#define SHOUT_META_DESCRIPTION      "description"
#define SHOUT_META_IRC              "irc"
#define SHOUT_META_AIM              "aim"
#define SHOUT_META_ICQ              "icq"

typedef struct shout shout_t;
typedef struct _util_dict shout_metadata_t;

#ifdef __cplusplus
extern "C" {
#endif

/* initializes the shout library. Must be called before anything else */
void shout_init(void);

/* shuts down the shout library, deallocating any global storage. Don't call
 * anything afterwards */
void shout_shutdown(void);

/* returns a static version string.  Non-null parameters will be set to the
 * value of the library major, minor, and patch levels, respectively */
const char *shout_version(int *major, int *minor, int *patch);

/* Allocates and sets up a new shout_t.  Returns NULL if it can't get enough
 * memory.  The returns shout_t must be disposed of with shout_free. */
shout_t *shout_new(void);

/* Free all memory allocated by a shout_t */
void shout_free(shout_t *self);

/* Returns a statically allocated string describing the last shout error
 * to occur.  Only valid until the next libshout call on this shout_t */
const char *shout_get_error(shout_t *self);

/* Return the error code (e.g. SHOUTERR_SOCKET) for this shout instance */
int shout_get_errno(shout_t *self);

/* returns SHOUTERR_CONNECTED or SHOUTERR_UNCONNECTED */
int shout_get_connected(shout_t *self);

/* Parameter manipulation functions.  libshout makes copies of all parameters,
 * the caller may free its copies after giving them to libshout.  May return
 * SHOUTERR_MALLOC */

/* Connection parameters */
int shout_set_host(shout_t *self, const char *host);
const char *shout_get_host(shout_t *self);

int shout_set_port(shout_t *self, unsigned short port);
unsigned short shout_get_port(shout_t *self);

int shout_set_agent(shout_t *self, const char *agent);
const char *shout_get_agent(shout_t *self);

/* See SHOUT_TLS_* above */
int shout_set_tls(shout_t *self, int mode);
int shout_get_tls(shout_t *self);

/* Set the directory for CA certs. Default: operating system's default */
int shout_set_ca_directory(shout_t *self, const char *directory);
const char *shout_get_ca_directory(shout_t *self);

/* Set a CA cert file for checking. If you use a self signed server cert
 * you can pass this cert using this function for verification.
 * Default: operating system's default */
int shout_set_ca_file(shout_t *self, const char *file);
const char *shout_get_ca_file(shout_t *self);

/* Set list of allowed ciphers.
 * This function should only be used in case of using an old libshout
 * after some attacks got known. Watch the icecast mailinglist for
 * known problems.
 * DO NOT SET THIS TO ANY FIXED VALUE. IF YOU USE THIS FUNCTION
 * EXPOSE IT TO THE USER. OTHERWISE YOU WILL HARM SECURITY.
 * Default: internal list of secure ciphers. */
int shout_set_allowed_ciphers(shout_t *self, const char *ciphers);
const char *shout_get_allowed_ciphers(shout_t *self);

/* Authentication parameters */
int shout_set_user(shout_t *self, const char *username);
const char *shout_get_user(shout_t *self);

int shout_set_password(shout_t *, const char *password);
const char *shout_get_password(shout_t *self);

/* Set a client certificate for TLS connections.
 * This must be in PEM format with both cert and private key in the same file.
 * Default: none. */
int shout_set_client_certificate(shout_t *self, const char *certificate);
const char *shout_get_client_certificate(shout_t *self);

/* Mount parameters */
int shout_set_mount(shout_t *self, const char *mount);
const char *shout_get_mount(shout_t *self);

/* Other parameters */
int shout_set_name(shout_t *self, const char *name); // obsolete
const char *shout_get_name(shout_t *self); // obsolete

int shout_set_url(shout_t *self, const char *url); // obsolete
const char *shout_get_url(shout_t *self); // obsolete

int shout_set_genre(shout_t *self, const char *genre); // obsolete
const char *shout_get_genre(shout_t *self); // obsolete

int shout_set_description(shout_t *self, const char *description); // obsolete
const char *shout_get_description(shout_t *self); // obsolete

int shout_set_dumpfile(shout_t *self, const char *dumpfile);
const char *shout_get_dumpfile(shout_t *self);

int shout_set_audio_info(shout_t *self, const char *name, const char *value);
const char *shout_get_audio_info(shout_t *self, const char *name);

/* takes a SHOUT_META_xxxx argument */
int shout_set_meta(shout_t *self, const char *name, const char *value);
const char *shout_get_meta(shout_t *self, const char *name);

int shout_set_public(shout_t *self, unsigned int make_public);
unsigned int shout_get_public(shout_t *self);

/* takes a SHOUT_FORMAT_xxxx argument */
int shout_set_format(shout_t *self, unsigned int format);
unsigned int shout_get_format(shout_t *self);

/* takes a SHOUT_PROTOCOL_xxxxx argument */
int shout_set_protocol(shout_t *self, unsigned int protocol);
unsigned int shout_get_protocol(shout_t *self);

/* Explicitly sets the mimetype. Set to NULL to revert back to the default.
   The default value is dependent on the format setting. */
int shout_set_mimetype(shout_t *self, const char *mimetype);

const char *shout_get_mimetype(shout_t *self);

/* Instructs libshout to use nonblocking I/O. Must be called before
 * shout_open (no switching back and forth midstream at the moment). */
int shout_set_nonblocking(shout_t* self, unsigned int nonblocking);
unsigned int shout_get_nonblocking(shout_t *self);

/* Opens a connection to the server.  All parameters must already be set */
int shout_open(shout_t *self);

/* Closes a connection to the server */
int shout_close(shout_t *self);

/* Send data to the server, parsing it for format specific timing info */
int shout_send(shout_t *self, const unsigned char *data, size_t len);

/* Send unparsed data to the server.  Do not use this unless you know
 * what you are doing. 
 * Returns the number of bytes written, or < 0 on error.
 */
ssize_t shout_send_raw(shout_t *self, const unsigned char *data, size_t len);

/* return the number of bytes currently on the write queue (only makes sense in
 * nonblocking mode). */
ssize_t shout_queuelen(shout_t *self);
  
/* Puts caller to sleep until it is time to send more data to the server */
void shout_sync(shout_t *self);

/* Amount of time in ms caller should wait before sending again */
int shout_delay(shout_t *self);

/* Sets MP3 metadata.
 * Returns:
 *   SHOUTERR_SUCCESS
 *   SHOUTERR_UNSUPPORTED if format isn't MP3
 *   SHOUTERR_MALLOC
 *   SHOUTERR_INSANE
 *   SHOUTERR_NOCONNECT
 *   SHOUTERR_SOCKET
 */
int shout_set_metadata(shout_t *self, shout_metadata_t *metadata);

/* Allocates a new metadata structure.  Must be freed by shout_metadata_free. */
shout_metadata_t *shout_metadata_new(void);

/* Free resources allocated by shout_metadata_t */
void shout_metadata_free(shout_metadata_t *self);

/* Add a parameter to the metadata structure.
 * Returns:
 *   SHOUTERR_SUCCESS on success
 *   SHOUTERR_INSANE if self isn't a valid shout_metadata_t* or name is null
 *   SHOUTERR_MALLOC if memory can't be allocated */
int shout_metadata_add(shout_metadata_t *self, const char *name, const char *value);

#ifdef __cplusplus
}
#endif

/* --- Compiled features --- */

#define SHOUT_THREADSAFE 1
#define SHOUT_TLS        1

#endif /* __LIBSHOUT_SHOUT_H__ */
