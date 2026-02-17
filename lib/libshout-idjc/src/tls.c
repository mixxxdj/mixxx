/* -*- c-basic-offset: 8; -*- */
/* tls.c: TLS support functions
 * $Id$
 *
 *  Copyright (C) 2015-2019 Philipp Schafft <lion@lion.leolix.org>
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

#include <string.h>

#include <openssl/x509v3.h>

#include <shoutidjc/shout.h>
#include "shout_private.h"

struct _shout_tls {
    SSL_CTX     *ssl_ctx;
    SSL         *ssl;
    int          ssl_ret;
    int          cert_error;
    /* only pointers into self, don't need to free them */
    sock_t       socket;
    const char  *host;
    const char  *ca_directory;
    const char  *ca_file;
    const char  *allowed_ciphers;
    const char  *client_certificate;
    shout_tls_callback_t callback;
    void        *callback_userdata;
};

static int shout_tls_emit(shout_tls_t *tls, shout_event_t event, ...)
{
    int ret;
    va_list ap;

    if (!tls)
        return SHOUTERR_INSANE;

    if (!tls->callback)
        return SHOUT_CALLBACK_PASS;

    va_start(ap, event);
    ret = tls->callback(tls, event, tls->callback_userdata, ap);
    va_end(ap);

    return ret;
}

shout_tls_t *shout_tls_new(shout_t *self, sock_t socket)
{
    shout_tls_t *tls = calloc(1, sizeof(shout_tls_t));
    if (!tls)
        return NULL;

    tls->cert_error = SHOUTERR_RETRY;

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
    long ssl_opts = 0;

#if OPENSSL_VERSION_NUMBER < 0x10100000L
    SSL_library_init();
    SSL_load_error_strings();
    SSLeay_add_all_algorithms();
    SSLeay_add_ssl_algorithms();

    tls->ssl_ctx = SSL_CTX_new(TLSv1_client_method());
    ssl_opts |= SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3; // Disable SSLv2 and SSLv3
#else
    tls->ssl_ctx = SSL_CTX_new(TLS_client_method());
    SSL_CTX_set_min_proto_version(tls->ssl_ctx, TLS1_VERSION);
#endif

#ifdef SSL_OP_NO_COMPRESSION
    ssl_opts |= SSL_OP_NO_COMPRESSION;             // Never use compression
#endif

    if (!tls->ssl_ctx)
        goto error;

    /* Even though this function is called set, it adds the
     * flags to the already existing flags (possibly default
     * flags already set by OpenSSL)!
     * Calling SSL_CTX_get_options is not needed here, therefore.
     */
    SSL_CTX_set_options(tls->ssl_ctx, ssl_opts);


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

static inline int tls_check_cert(shout_tls_t *tls)
{
    X509 *cert = SSL_get_peer_certificate(tls->ssl);
    int cert_ok = 0;
    int ret;

    if (tls->cert_error != SHOUTERR_RETRY && tls->cert_error != SHOUTERR_BUSY)
        return tls->cert_error;

    if (!cert)
        return SHOUTERR_TLSBADCERT;

    ret = shout_tls_emit(tls, SHOUT_EVENT_TLS_CHECK_PEER_CERTIFICATE);
    if (ret != SHOUT_CALLBACK_PASS)
        return tls->cert_error = ret;

    do {
        if (SSL_get_verify_result(tls->ssl) != X509_V_OK)
            break;

        if (X509_check_host(cert, tls->host, 0, 0, NULL) != 1)
            break;

        /* ok, all test passed... */
        cert_ok = 1;
    } while (0);

    X509_free(cert);

    if (cert_ok) {
        tls->cert_error = SHOUTERR_SUCCESS;
    } else {
        tls->cert_error = SHOUTERR_TLSBADCERT;
    }
    return tls->cert_error;
}

static inline int tls_setup_process(shout_tls_t *tls)
{
    if (SSL_is_init_finished(tls->ssl))
        return tls_check_cert(tls);
    tls->ssl_ret = SSL_connect(tls->ssl);
    if (SSL_is_init_finished(tls->ssl))
        return tls_check_cert(tls);
    if (!shout_tls_recoverable(tls))
        return SHOUTERR_SOCKET;
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

int          shout_tls_get_peer_certificate(shout_tls_t *tls, char **buf)
{
    X509 *cert;
    BIO *bio;
    unsigned char *data;
    unsigned int len;


    if (!tls || !buf)
        return SHOUTERR_INSANE;

    cert  = SSL_get_peer_certificate(tls->ssl);
    if (!cert)
        return SHOUTERR_TLSBADCERT;

    bio = BIO_new(BIO_s_mem());
    if (!bio)
        return SHOUTERR_MALLOC;

    PEM_write_bio_X509(bio, cert);

    len = BIO_get_mem_data(bio, &data);

    if (len) {
        *buf = malloc(len + 1);
        memcpy(*buf, data, len);
        (*buf)[len] = 0;
    }

    BIO_free(bio);

    return SHOUTERR_SUCCESS;
}

int          shout_tls_get_peer_certificate_chain(shout_tls_t *tls, char **buf)
{
    BIO *bio;
    unsigned char *data;
    unsigned int len;
    int j, certs;
    STACK_OF(X509) * chain;


    if (!tls || !buf)
        return SHOUTERR_INSANE;

    chain = SSL_get_peer_cert_chain(tls->ssl);

    certs = sk_X509_num(chain);

    if (!certs)
        return SHOUTERR_TLSBADCERT;

    bio = BIO_new(BIO_s_mem());
    if (!bio)
        return SHOUTERR_MALLOC;

    for(j = 0; j < certs; ++j) {
        X509 *cert = sk_X509_value(chain, j);

        PEM_write_bio_X509(bio, cert);

    }

    len = BIO_get_mem_data(bio, &data);

    if (len) {
        *buf = malloc(len + 1);
        memcpy(*buf, data, len);
        (*buf)[len] = 0;
    }

    BIO_free(bio);

    return SHOUTERR_SUCCESS;
}

int          shout_tls_set_callback(shout_tls_t *tls, shout_tls_callback_t callback, void *userdata)
{
    if (!tls)
        return SHOUTERR_INSANE;

    tls->callback = callback;
    tls->callback_userdata = userdata;

    return SHOUTERR_SUCCESS;
}
