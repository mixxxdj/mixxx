/*
 * Copyright 2004-2005 Timo Hirvonen
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef _IP_H
#define _IP_H

#include "comment.h"
#include "sf.h"

enum {
	/* no error */
	IP_ERROR_SUCCESS,
	/* system error (error code in errno) */
	IP_ERROR_ERRNO,
	/* file type not supported */
	IP_ERROR_UNRECOGNIZED_FILE_TYPE,
	/* function not supported (usually seek) */
	IP_ERROR_FUNCTION_NOT_SUPPORTED,
	/* input plugin detected corrupted file */
	IP_ERROR_FILE_FORMAT,
	/* malformed uri */
	IP_ERROR_INVALID_URI,
	/* sample format not supported */
	IP_ERROR_SAMPLE_FORMAT,
	/* error parsing response line / headers */
	IP_ERROR_HTTP_RESPONSE,
	/* usually 404 */
	IP_ERROR_HTTP_STATUS,
	/*  */
	IP_ERROR_INTERNAL
};

/*
struct input_plugin_data {
	// filled by ip-layer
	char *filename;
	int fd;

	unsigned int remote : 1;
	unsigned int metadata_changed : 1;

	// shoutcast
	int counter;
	int metaint;
	char *metadata;

	// filled by plugin
	sample_format_t sf;
	void * private_ipd;
};
*/

struct input_plugin_data {
	// filled by ip-layer
//	QString filename;
	char *filename;
	int fd;

	unsigned int remote : 1;
	unsigned int metadata_changed : 1;

	// shoutcast
	int counter;
	int metaint;
	char *metadata;

	// filled by plugin
	sample_format_t sf;
	void * private_ipd;
};


struct input_plugin_ops {
	int (*open)(struct input_plugin_data *ip_data);
	int (*close)(struct input_plugin_data *ip_data);
	int (*read)(struct input_plugin_data *ip_data, char *buffer, int count);
	int (*seek)(struct input_plugin_data *ip_data, double offset);
	int (*read_comments)(struct input_plugin_data *ip_data,
			struct keyval **comments);
	int (*duration)(struct input_plugin_data *ip_data);
};

#endif
