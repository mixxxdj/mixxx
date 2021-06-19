/* Httpp.c
**
** http parsing engine
**
** Copyright (C) 2014 Michael Smith <msmith@icecast.org>,
**                    Ralph Giles <giles@xiph.org>,
**                    Ed "oddsock" Zaleski <oddsock@xiph.org>,
**                    Karl Heyes <karl@xiph.org>,
**                    Philipp "ph3-der-loewe" Schafft <lion@lion.leolix.org>
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

#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <avl/avl.h>
#include "httpp.h"

#if _WIN32
#include <os.h>
#endif

#define MAX_HEADERS 32

/* internal functions */

/* misc */
static char *_lowercase(char *str);

/* for avl trees */
static int _compare_vars(void *compare_arg, void *a, void *b);
static int _free_vars(void *key);

http_parser_t *httpp_create_parser(void)
{
    return (http_parser_t *)malloc(sizeof(http_parser_t));
}

void httpp_initialize(http_parser_t *parser, http_varlist_t *defaults)
{
    http_varlist_t *list;

    parser->req_type = httpp_req_none;
    parser->uri = NULL;
    parser->vars = avl_tree_new(_compare_vars, NULL);
    parser->queryvars = avl_tree_new(_compare_vars, NULL);

    /* now insert the default variables */
    list = defaults;
    while (list != NULL) {
        httpp_setvar(parser, list->var.name, list->var.value);
        list = list->next;
    }
}

static int split_headers(char *data, unsigned long len, char **line)
{
    /* first we count how many lines there are 
    ** and set up the line[] array     
    */
    int lines = 0;
    unsigned long i;
    line[lines] = data;
    for (i = 0; i < len && lines < MAX_HEADERS; i++) {
        if (data[i] == '\r')
            data[i] = '\0';
        if (data[i] == '\n') {
            lines++;
            data[i] = '\0';
            if (lines >= MAX_HEADERS)
                return MAX_HEADERS;
            if (i + 1 < len) {
                if (data[i + 1] == '\n' || data[i + 1] == '\r')
                    break;
                line[lines] = &data[i + 1];
            }
        }
    }

    i++;
    while (i < len && data[i] == '\n') i++;

    return lines;
}

static void parse_headers(http_parser_t *parser, char **line, int lines)
{
    int i, l;
    int whitespace, slen;
    char *name = NULL;
    char *value = NULL;

    /* parse the name: value lines. */
    for (l = 1; l < lines; l++) {
        whitespace = 0;
        name = line[l];
        value = NULL;
        slen = strlen(line[l]);
        for (i = 0; i < slen; i++) {
            if (line[l][i] == ':') {
                whitespace = 1;
                line[l][i] = '\0';
            } else {
                if (whitespace) {
                    whitespace = 0;
                    while (i < slen && line[l][i] == ' ')
                        i++;

                    if (i < slen)
                        value = &line[l][i];
                    
                    break;
                }
            }
        }
        
        if (name != NULL && value != NULL) {
            httpp_setvar(parser, _lowercase(name), value);
            name = NULL; 
            value = NULL;
        }
    }
}

int httpp_parse_response(http_parser_t *parser, const char *http_data, unsigned long len, const char *uri)
{
    char *data;
    char *line[MAX_HEADERS];
    int lines, slen,i, whitespace=0, where=0,code;
    char *version=NULL, *resp_code=NULL, *message=NULL;
    
    if(http_data == NULL)
        return 0;

    /* make a local copy of the data, including 0 terminator */
    data = (char *)malloc(len+1);
    if (data == NULL) return 0;
    memcpy(data, http_data, len);
    data[len] = 0;

    lines = split_headers(data, len, line);

    /* In this case, the first line contains:
     * VERSION RESPONSE_CODE MESSAGE, such as HTTP/1.0 200 OK
     */
    slen = strlen(line[0]);
    version = line[0];
    for(i=0; i < slen; i++) {
        if(line[0][i] == ' ') {
            line[0][i] = 0;
            whitespace = 1;
        } else if(whitespace) {
            whitespace = 0;
            where++;
            if(where == 1)
                resp_code = &line[0][i];
            else {
                message = &line[0][i];
                break;
            }
        }
    }

    if(version == NULL || resp_code == NULL || message == NULL) {
        free(data);
        return 0;
    }

    httpp_setvar(parser, HTTPP_VAR_ERROR_CODE, resp_code);
    code = atoi(resp_code);
    if(code < 200 || code >= 300) {
        httpp_setvar(parser, HTTPP_VAR_ERROR_MESSAGE, message);
    }

    httpp_setvar(parser, HTTPP_VAR_URI, uri);
    httpp_setvar(parser, HTTPP_VAR_REQ_TYPE, "NONE");

    parse_headers(parser, line, lines);

    free(data);

    return 1;
}

static int hex(char c)
{
    if(c >= '0' && c <= '9')
        return c - '0';
    else if(c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    else if(c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    else
        return -1;
}

static char *url_escape(const char *src)
{
    int len = strlen(src);
    unsigned char *decoded;
    int i;
    char *dst;
    int done = 0;

    decoded = calloc(1, len + 1);

    dst = (char *)decoded;

    for(i=0; i < len; i++) {
        switch(src[i]) {
        case '%':
            if(i+2 >= len) {
                free(decoded);
                return NULL;
            }
            if(hex(src[i+1]) == -1 || hex(src[i+2]) == -1 ) {
                free(decoded);
                return NULL;
            }

            *dst++ = hex(src[i+1]) * 16  + hex(src[i+2]);
            i+= 2;
            break;
        case '+':
            *dst++ = ' ';
            break;
        case '#':
            done = 1;
            break;
        case 0:
            free(decoded);
            return NULL;
            break;
        default:
            *dst++ = src[i];
            break;
        }
        if(done)
            break;
    }

    *dst = 0; /* null terminator */

    return (char *)decoded;
}

/** TODO: This is almost certainly buggy in some cases */
static void parse_query(http_parser_t *parser, char *query)
{
    int len;
    int i=0;
    char *key = query;
    char *val=NULL;

    if(!query || !*query)
        return;

    len = strlen(query);

    while(i<len) {
        switch(query[i]) {
        case '&':
            query[i] = 0;
            if(val && key)
                httpp_set_query_param(parser, key, val);
            key = query+i+1;
            break;
        case '=':
            query[i] = 0;
            val = query+i+1;
            break;
        }
        i++;
    }

    if(val && key) {
        httpp_set_query_param(parser, key, val);
    }
}

int httpp_parse(http_parser_t *parser, const char *http_data, unsigned long len)
{
    char *data, *tmp;
    char *line[MAX_HEADERS]; /* limited to 32 lines, should be more than enough */
    int i;
    int lines;
    char *req_type = NULL;
    char *uri = NULL;
    char *version = NULL;
    int whitespace, where, slen;

    if (http_data == NULL)
        return 0;

    /* make a local copy of the data, including 0 terminator */
    data = (char *)malloc(len+1);
    if (data == NULL) return 0;
    memcpy(data, http_data, len);
    data[len] = 0;

    lines = split_headers(data, len, line);

    /* parse the first line special
    ** the format is:
    ** REQ_TYPE URI VERSION
    ** eg:
    ** GET /index.html HTTP/1.0
    */
    where = 0;
    whitespace = 0;
    slen = strlen(line[0]);
    req_type = line[0];
    for (i = 0; i < slen; i++) {
        if (line[0][i] == ' ') {
            whitespace = 1;
            line[0][i] = '\0';
        } else {
            /* we're just past the whitespace boundry */
            if (whitespace) {
                whitespace = 0;
                where++;
                switch (where) {
                case 1:
                    uri = &line[0][i];
                    break;
                case 2:
                    version = &line[0][i];
                    break;
                }
            }
        }
    }

    parser->req_type = httpp_str_to_method(req_type);

    if (uri != NULL && strlen(uri) > 0) {
        char *query;
        if((query = strchr(uri, '?')) != NULL) {
            httpp_setvar(parser, HTTPP_VAR_RAWURI, uri);
            httpp_setvar(parser, HTTPP_VAR_QUERYARGS, query);
            *query = 0;
            query++;
            parse_query(parser, query);
        }

        parser->uri = strdup(uri);
    } else {
        free(data);
        return 0;
    }

    if ((version != NULL) && ((tmp = strchr(version, '/')) != NULL)) {
        tmp[0] = '\0';
        if ((strlen(version) > 0) && (strlen(&tmp[1]) > 0)) {
            httpp_setvar(parser, HTTPP_VAR_PROTOCOL, version);
            httpp_setvar(parser, HTTPP_VAR_VERSION, &tmp[1]);
        } else {
            free(data);
            return 0;
        }
    } else {
        free(data);
        return 0;
    }

    if (parser->req_type != httpp_req_none && parser->req_type != httpp_req_unknown) {
        switch (parser->req_type) {
        case httpp_req_get:
            httpp_setvar(parser, HTTPP_VAR_REQ_TYPE, "GET");
            break;
        case httpp_req_post:
            httpp_setvar(parser, HTTPP_VAR_REQ_TYPE, "POST");
            break;
        case httpp_req_put:
            httpp_setvar(parser, HTTPP_VAR_REQ_TYPE, "PUT");
            break;
        case httpp_req_head:
            httpp_setvar(parser, HTTPP_VAR_REQ_TYPE, "HEAD");
            break;
        case httpp_req_options:
            httpp_setvar(parser, HTTPP_VAR_REQ_TYPE, "OPTIONS");
            break;
        case httpp_req_delete:
            httpp_setvar(parser, HTTPP_VAR_REQ_TYPE, "DELETE");
            break;
        case httpp_req_trace:
            httpp_setvar(parser, HTTPP_VAR_REQ_TYPE, "TRACE");
            break;
        case httpp_req_connect:
            httpp_setvar(parser, HTTPP_VAR_REQ_TYPE, "CONNECT");
            break;
        case httpp_req_source:
            httpp_setvar(parser, HTTPP_VAR_REQ_TYPE, "SOURCE");
            break;
        case httpp_req_play:
            httpp_setvar(parser, HTTPP_VAR_REQ_TYPE, "PLAY");
            break;
        case httpp_req_stats:
            httpp_setvar(parser, HTTPP_VAR_REQ_TYPE, "STATS");
            break;
        default:
            break;
        }
    } else {
        free(data);
        return 0;
    }

    if (parser->uri != NULL) {
        httpp_setvar(parser, HTTPP_VAR_URI, parser->uri);
    } else {
        free(data);
        return 0;
    }

    parse_headers(parser, line, lines);

    free(data);

    return 1;
}

void httpp_deletevar(http_parser_t *parser, const char *name)
{
    http_var_t var;

    if (parser == NULL || name == NULL)
        return;
    var.name = (char*)name;
    var.value = NULL;
    avl_delete(parser->vars, (void *)&var, _free_vars);
}

void httpp_setvar(http_parser_t *parser, const char *name, const char *value)
{
    http_var_t *var;

    if (name == NULL || value == NULL)
        return;

    var = (http_var_t *)malloc(sizeof(http_var_t));
    if (var == NULL) return;

    var->name = strdup(name);
    var->value = strdup(value);

    if (httpp_getvar(parser, name) == NULL) {
        avl_insert(parser->vars, (void *)var);
    } else {
        avl_delete(parser->vars, (void *)var, _free_vars);
        avl_insert(parser->vars, (void *)var);
    }
}

const char *httpp_getvar(http_parser_t *parser, const char *name)
{
    http_var_t var;
    http_var_t *found;
    void *fp;

    if (parser == NULL || name == NULL)
        return NULL;

    fp = &found;
    var.name = (char*)name;
    var.value = NULL;

    if (avl_get_by_key(parser->vars, &var, fp) == 0)
        return found->value;
    else
        return NULL;
}

void httpp_set_query_param(http_parser_t *parser, const char *name, const char *value)
{
    http_var_t *var;

    if (name == NULL || value == NULL)
        return;

    var = (http_var_t *)malloc(sizeof(http_var_t));
    if (var == NULL) return;

    var->name = strdup(name);
    var->value = url_escape(value);

    if (httpp_get_query_param(parser, name) == NULL) {
        avl_insert(parser->queryvars, (void *)var);
    } else {
        avl_delete(parser->queryvars, (void *)var, _free_vars);
        avl_insert(parser->queryvars, (void *)var);
    }
}

const char *httpp_get_query_param(http_parser_t *parser, const char *name)
{
    http_var_t var;
    http_var_t *found;
    void *fp;

    fp = &found;
    var.name = (char *)name;
    var.value = NULL;

    if (avl_get_by_key(parser->queryvars, (void *)&var, fp) == 0)
        return found->value;
    else
        return NULL;
}

void httpp_clear(http_parser_t *parser)
{
    parser->req_type = httpp_req_none;
    if (parser->uri)
        free(parser->uri);
    parser->uri = NULL;
    avl_tree_free(parser->vars, _free_vars);
    avl_tree_free(parser->queryvars, _free_vars);
    parser->vars = NULL;
}

void httpp_destroy(http_parser_t *parser)
{
    httpp_clear(parser);
    free(parser);
}

static char *_lowercase(char *str)
{
    char *p = str;
    for (; *p != '\0'; p++)
        *p = tolower(*p);

    return str;
}

static int _compare_vars(void *compare_arg, void *a, void *b)
{
    http_var_t *vara, *varb;

    vara = (http_var_t *)a;
    varb = (http_var_t *)b;

    return strcmp(vara->name, varb->name);
}

static int _free_vars(void *key)
{
    http_var_t *var;

    var = (http_var_t *)key;

    if (var->name)
        free(var->name);
    if (var->value)
        free(var->value);
    free(var);

    return 1;
}

httpp_request_type_e httpp_str_to_method(const char * method) {
    if (strcasecmp("GET", method) == 0) {
        return httpp_req_get;
    } else if (strcasecmp("POST", method) == 0) {
        return httpp_req_post;
    } else if (strcasecmp("PUT", method) == 0) {
        return httpp_req_put;
    } else if (strcasecmp("HEAD", method) == 0) {
        return httpp_req_head;
    } else if (strcasecmp("OPTIONS", method) == 0) {
        return httpp_req_options;
    } else if (strcasecmp("DELETE", method) == 0) {
        return httpp_req_delete;
    } else if (strcasecmp("TRACE", method) == 0) {
        return httpp_req_trace;
    } else if (strcasecmp("CONNECT", method) == 0) {
        return httpp_req_connect;
    } else if (strcasecmp("SOURCE", method) == 0) {
        return httpp_req_source;
    } else if (strcasecmp("PLAY", method) == 0) {
        return httpp_req_play;
    } else if (strcasecmp("STATS", method) == 0) {
        return httpp_req_stats;
    } else {
        return httpp_req_unknown;
    }
}

