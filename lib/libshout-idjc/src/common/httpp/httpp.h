/* httpp.h
**
** http parsing library
**
** Copyright (C) 2014 Michael Smith <msmith@icecast.org>,
**                    Ralph Giles <giles@xiph.org>,
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

#ifndef __HTTPP_H
#define __HTTPP_H

#include <avl/avl.h>

#define HTTPP_VAR_PROTOCOL "__protocol"
#define HTTPP_VAR_VERSION "__version"
#define HTTPP_VAR_URI "__uri"
#define HTTPP_VAR_RAWURI "__rawuri"
#define HTTPP_VAR_QUERYARGS " __queryargs"
#define HTTPP_VAR_REQ_TYPE "__req_type"
#define HTTPP_VAR_ERROR_MESSAGE "__errormessage"
#define HTTPP_VAR_ERROR_CODE "__errorcode"
#define HTTPP_VAR_ICYPASSWORD "__icy_password"

typedef enum httpp_request_type_tag {
    /* Initial and internally used state of the engine */
    httpp_req_none = 0,
    /* Part of HTTP standard: GET, POST, PUT and HEAD */
    httpp_req_get,
    httpp_req_post,
    httpp_req_put,
    httpp_req_head,
    httpp_req_options,
    httpp_req_delete,
    httpp_req_trace,
    httpp_req_connect,
    /* Icecast SOURCE, to be replaced with PUT some day */
    httpp_req_source,
    /* XXX: ??? */
    httpp_req_play,
    /* Icecast 2.x STATS, to request a live stream of stats events */
    httpp_req_stats,
    /* Used if request method is unknown. MUST BE LAST ONE IN LIST. */
    httpp_req_unknown
} httpp_request_type_e;

typedef struct http_var_tag {
    char *name;
    char *value;
} http_var_t;

typedef struct http_varlist_tag {
    http_var_t var;
    struct http_varlist_tag *next;
} http_varlist_t;

typedef struct http_parser_tag {
    httpp_request_type_e req_type;
    char *uri;
    avl_tree *vars;
    avl_tree *queryvars;
} http_parser_t;

#ifdef _mangle
# define httpp_create_parser _mangle(httpp_create_parser)
# define httpp_initialize _mangle(httpp_initialize)
# define httpp_parse _mangle(httpp_parse)
# define httpp_parse_icy _mangle(httpp_parse_icy)
# define httpp_parse_response _mangle(httpp_parse_response)
# define httpp_setvar _mangle(httpp_setvar)
# define httpp_getvar _mangle(httpp_getvar)
# define httpp_set_query_param _mangle(httpp_set_query_param)
# define httpp_get_query_param _mangle(httpp_get_query_param)
# define httpp_destroy _mangle(httpp_destroy)
# define httpp_clear _mangle(httpp_clear)
#endif

http_parser_t *httpp_create_parser(void);
void httpp_initialize(http_parser_t *parser, http_varlist_t *defaults);
int httpp_parse(http_parser_t *parser, const char *http_data, unsigned long len);
int httpp_parse_icy(http_parser_t *parser, const char *http_data, unsigned long len);
int httpp_parse_response(http_parser_t *parser, const char *http_data, unsigned long len, const char *uri);
void httpp_setvar(http_parser_t *parser, const char *name, const char *value);
void httpp_deletevar(http_parser_t *parser, const char *name);
const char *httpp_getvar(http_parser_t *parser, const char *name);
void httpp_set_query_param(http_parser_t *parser, const char *name, const char *value);
const char *httpp_get_query_param(http_parser_t *parser, const char *name);
void httpp_destroy(http_parser_t *parser);
void httpp_clear(http_parser_t *parser);

/* util functions */
httpp_request_type_e httpp_str_to_method(const char * method);
 
#endif
