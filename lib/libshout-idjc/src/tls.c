/* -*- c-basic-offset: 8; -*- */
/* tls.c: TLS support functions
 * $Id$
 *
 *  Copyright (C) 2015 Philipp Schafft <lion@lion.leolix.org>
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

#include <shoutidjc/shout.h>
#include <string.h>
#include "shout_private.h"

#ifndef XXX_HAVE_X509_check_host
#   include <ctype.h>
#endif

struct _shout_tls {
    SSL_CTX     *ssl_ctx;
    SSL         *ssl;
    int          ssl_ret;
    /* only pointers into self, don't need to free them */
    sock_t       socket;
    const char  *host;
    const char  *ca_directory;
    const char  *ca_file;
    const char  *allowed_ciphers;
    const char  *client_certificate;
};

shout_tls_t *shout_tls_new(shout_t *self, sock_t socket)
{
    shout_tls_t *tls = calloc(1, sizeof(shout_tls_t));
    if (!tls)
        return NULL;

    tls->socket             = socket;
    tls->host               = self->host;
    tls->ca_directory       = self->ca_directory;
    tls->ca_file            = self->ca_file;
    tls->allowed_ciphers    = self->allowed_ciphers;
    tls->client_certificate = self->client_certificate;

    return tls;
}

static inline int tls_setup(shout_tls_t *tls)
{
    const SSL_METHOD *meth;
#if (OPENSSL_VERSION_NUMBER < 0x10100000L) || defined(LIBRESSL_VERSION_NUMBER)
    SSL_library_init();
    SSL_load_error_strings();
    SSLeay_add_all_algorithms();
    SSLeay_add_ssl_algorithms();

    meth = SSLv23_client_method();
#else
    meth = TLS_client_method();
#endif
    if (!meth)
        goto error;

    tls->ssl_ctx = SSL_CTX_new(meth);
    if (!tls->ssl_ctx)
        goto error;

    SSL_CTX_set_default_verify_paths(tls->ssl_ctx);
    SSL_CTX_load_verify_locations(tls->ssl_ctx, tls->ca_file, tls->ca_directory);

    SSL_CTX_set_verify(tls->ssl_ctx, SSL_VERIFY_NONE, NULL);

    if (tls->client_certificate) {
        if (SSL_CTX_use_certificate_file(tls->ssl_ctx, tls->client_certificate, SSL_FILETYPE_PEM) != 1)
            goto error;
        if (SSL_CTX_use_PrivateKey_file(tls->ssl_ctx, tls->client_certificate, SSL_FILETYPE_PEM) != 1)
            goto error;
        }

    if (SSL_CTX_set_cipher_list(tls->ssl_ctx, tls->allowed_ciphers) <= 0)
        goto error;

    SSL_CTX_set_mode(tls->ssl_ctx, SSL_MODE_ENABLE_PARTIAL_WRITE);
    SSL_CTX_set_mode(tls->ssl_ctx, SSL_MODE_AUTO_RETRY);

    tls->ssl = SSL_new(tls->ssl_ctx);
    if (!tls->ssl)
        goto error;

    if (!SSL_set_fd(tls->ssl, tls->socket))
        goto error;

    SSL_set_tlsext_host_name(tls->ssl, tls->host);
    SSL_set_connect_state(tls->ssl);
    tls->ssl_ret = SSL_connect(tls->ssl);

    return SHOUTERR_SUCCESS;

error:
    if (tls->ssl)
        SSL_free(tls->ssl);
    if (tls->ssl_ctx)
        SSL_CTX_free(tls->ssl_ctx);
    return SHOUTERR_UNSUPPORTED;
}

#ifndef XXX_HAVE_X509_check_host
static inline int tls_check_pattern(const char *key, const char *pattern)
{
    for (; *key && *pattern; key++) {
        if (*pattern == '*') {
            for (; *pattern == '*'; pattern++) ;
            for (; *key && *key != '.'; key++) ;
            if (!*pattern && !*key)
                return 1;
            if (!*pattern || !*key)
                return 0;
        }

        if (tolower(*key) != tolower(*pattern))
            return 0;
        pattern++;
    }
    return *key == 0 && *pattern == 0;
}

static inline int tls_check_host(X509 *cert, const char *hostname)
{
    char common_name[256] = "";
    X509_NAME *xname = X509_get_subject_name(cert);
    X509_NAME_ENTRY *xentry;
    ASN1_STRING *sdata;
    int i, j;
    int ret;

    ret = X509_NAME_get_text_by_NID(xname, NID_commonName, common_name, sizeof(common_name));
    if (ret < 1 || (size_t)ret >= (sizeof(common_name)-1))
        return SHOUTERR_TLSBADCERT;

    if (!tls_check_pattern(hostname, common_name))
        return SHOUTERR_TLSBADCERT;

    /* check for inlined \0, 
     * see https://www.blackhat.com/html/bh-usa-09/bh-usa-09-archives.html#Marlinspike 
     */
    for (i = -1; ; i = j) {
        j = X509_NAME_get_index_by_NID(xname, NID_commonName, i);
        if (j == -1)
            break;
    }

    xentry = X509_NAME_get_entry(xname, i);
    sdata = X509_NAME_ENTRY_get_data(xentry);

    if ((size_t)ASN1_STRING_length(sdata) != strlen(common_name))
        return SHOUTERR_TLSBADCERT;

    return SHOUTERR_SUCCESS;
}
#endif

static inline int tls_check_cert(shout_tls_t *tls)
{
    X509 *cert = SSL_get_peer_certificate(tls->ssl);
    int cert_ok = 0;
    if (!cert)
        return SHOUTERR_TLSBADCERT;

    do {
        if (SSL_get_verify_result(tls->ssl) != X509_V_OK)
            break;

#ifdef XXX_HAVE_X509_check_host
        if (X509_check_host(cert, tls->host, 0, 0, NULL) != 1)
            break;
#else
        if (tls_check_host(cert, tls->host) != SHOUTERR_SUCCESS)
            break;
#endif

        /* ok, all test passed... */
        cert_ok = 1;
    } while (0);

    X509_free(cert);
    return cert_ok ? SHOUTERR_SUCCESS : SHOUTERR_TLSBADCERT;
}

static inline int tls_setup_process(shout_tls_t *tls)
{
    if (SSL_is_init_finished(tls->ssl))
        return tls_check_cert(tls);
    tls->ssl_ret = SSL_connect(tls->ssl);
    if (SSL_is_init_finished(tls->ssl))
        return tls_check_cert(tls);
    return SHOUTERR_BUSY;
}

int shout_tls_try_connect(shout_tls_t *tls)
{
    if (!tls->ssl)
        tls_setup(tls);
    if (tls->ssl)
        return tls_setup_process(tls);
    return SHOUTERR_UNSUPPORTED;
}

int shout_tls_close(shout_tls_t *tls)
{
    if (tls->ssl) {
        SSL_shutdown(tls->ssl);
        SSL_free(tls->ssl);
    }
    if (tls->ssl_ctx)
        SSL_CTX_free(tls->ssl_ctx);
    free(tls);
    return SHOUTERR_SUCCESS;
}

ssize_t shout_tls_read(shout_tls_t *tls, void *buf, size_t len)
{
    return tls->ssl_ret = SSL_read(tls->ssl, buf, len);
}

ssize_t shout_tls_write(shout_tls_t *tls, const void *buf, size_t len)
{
    return tls->ssl_ret = SSL_write(tls->ssl, buf, len);
}

int shout_tls_recoverable(shout_tls_t *tls)
{
    int error = SSL_get_error(tls->ssl, tls->ssl_ret);
    if (error == SSL_ERROR_WANT_READ || error == SSL_ERROR_WANT_WRITE)
        return 1;
    return 0;
}
