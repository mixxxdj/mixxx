/*
** Copyright (C) 2002,2003 Erik de Castro Lopo <erikd@mega-nerd.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>

#include	"config.h"

#include	"samplerate.h"
#include	"common.h"

SRC_STATE *
src_new (int converter_type, int channels, int *error)
{	SRC_PRIVATE	*psrc ;

	if (error)
		*error = SRC_ERR_NO_ERROR ;

	if (channels < 1)
	{	if (error)
			*error = SRC_ERR_BAD_CHANNEL_COUNT ;
		return NULL ;
		} ;

	if ((psrc = calloc (1, sizeof (*psrc))) == NULL)
	{	if (error)
			*error = SRC_ERR_MALLOC_FAILED ;
		return NULL ;
		} ;

	psrc->channels = channels ;

	if (sinc_set_converter (psrc, converter_type) != SRC_ERR_NO_ERROR &&
			zoh_set_converter (psrc, converter_type) != SRC_ERR_NO_ERROR &&
				linear_set_converter (psrc, converter_type) != SRC_ERR_NO_ERROR)
	{	if (error)
			*error = SRC_ERR_BAD_CONVERTER ;
		free (psrc) ;
		psrc = NULL ;
		} ;

	src_reset ((SRC_STATE*) psrc) ;

	return (SRC_STATE*) psrc ;
} /* src_new */

SRC_STATE *
src_delete (SRC_STATE *state)
{	SRC_PRIVATE *psrc ;

	psrc = (SRC_PRIVATE*) state ;
	if (psrc)
	{	if (psrc->private_data)
			free (psrc->private_data) ;
		memset (psrc, 0, sizeof (SRC_PRIVATE)) ;
		free (psrc) ;
		} ;

	return NULL ;
} /* src_state */

int
src_process (SRC_STATE *state, SRC_DATA *data)
{	SRC_PRIVATE *psrc ;
	int error ;

	psrc = (SRC_PRIVATE*) state ;

	if (psrc == NULL)
		return SRC_ERR_BAD_STATE ;
	if (psrc->process == NULL)
		return SRC_ERR_BAD_PROC_PTR ;

	/* Check for valid SRC_DATA first. */
	if (data == NULL)
		return SRC_ERR_BAD_DATA ;
	/* Check src_ratio is in range. */
	if (data->src_ratio < (1.0 / SRC_MAX_RATIO) || data->src_ratio > (1.0 * SRC_MAX_RATIO))
		return SRC_ERR_BAD_SRC_RATIO ;

	/* And that data_in and data_out are valid. */
	if (data->data_in == NULL || data->data_out == NULL)
		return SRC_ERR_BAD_DATA_PTR ;

	if (data->data_in == NULL)
		data->input_frames = 0 ;
	
	if (data->data_in < data->data_out)
	{	if (data->data_in + data->input_frames * psrc->channels > data->data_out)
		{	/*-printf ("data_in: %p    data_out: %p\n", 
				data->data_in + data->input_frames * psrc->channels, data->data_out) ;-*/
			return SRC_ERR_DATA_OVERLAP ;
			} ;
		}
	else if (data->data_out + data->output_frames * psrc->channels > data->data_in)
	{	/*-printf ("data_out: %p (%p)    data_in: %p\n", data->data_out, 
			data->data_out + data->output_frames * psrc->channels, data->data_in) ;-*/
		return SRC_ERR_DATA_OVERLAP ;
		} ;

	if (data->input_frames < 0)
		data->input_frames = 0 ;
	if (data->output_frames < 0)
		data->output_frames = 0 ;

	/* Set the input and output counts to zero. */
	data->input_frames_used = 0 ;
	data->output_frames_gen = 0 ;

	/* Now process. */
	error = psrc->process (state, data) ;

	return error ;
} /* src_process */

int
src_set_ratio (SRC_STATE *state, double new_ratio)
{	SRC_PRIVATE *psrc ;

	psrc = (SRC_PRIVATE*) state ;

	if (psrc == NULL)
		return SRC_ERR_BAD_STATE ;
	if (psrc->process == NULL)
		return SRC_ERR_BAD_PROC_PTR ;

	psrc->last_ratio = new_ratio ;

	return SRC_ERR_NO_ERROR ;
} /* src_set_ratio */

int
src_reset (SRC_STATE *state)
{	SRC_PRIVATE *psrc ;

	if ((psrc = (SRC_PRIVATE*) state) == NULL)
		return SRC_ERR_BAD_STATE ;

	if (psrc->reset != NULL)
		psrc->reset (psrc) ;

	psrc->last_position = 0.0 ;
	psrc->last_ratio = 0.0 ;

	psrc->error = SRC_ERR_NO_ERROR ;

	return SRC_ERR_NO_ERROR ;
} /* src_reset */

/*==============================================================================
**	Control functions.
*/

const char *
src_get_name (int converter_type)
{	const char *desc ;

	if ((desc = sinc_get_name (converter_type)) != NULL)
		return desc ;

	if ((desc = zoh_get_name (converter_type)) != NULL)
		return desc ;

	if ((desc = linear_get_name (converter_type)) != NULL)
		return desc ;

	return NULL ;
} /* src_get_name */

const char *
src_get_description (int converter_type)
{	const char *desc ;

	if ((desc = sinc_get_description (converter_type)) != NULL)
		return desc ;

	if ((desc = zoh_get_description (converter_type)) != NULL)
		return desc ;

	if ((desc = linear_get_description (converter_type)) != NULL)
		return desc ;

	return NULL ;
} /* src_get_description */

const char *
src_get_version (void)
{	return PACKAGE "-" VERSION ;
} /* src_get_version */

int
src_is_valid_ratio (double ratio)
{
	if (ratio < (1.0 / SRC_MAX_RATIO) || ratio > (1.0 * SRC_MAX_RATIO))
		return SRC_FALSE ;

	return SRC_TRUE ;
} /* src_is_valid_ratio */

/*==============================================================================
**	Error reporting functions.
*/

int
src_error (SRC_STATE *state)
{	if (state)
		return ((SRC_PRIVATE*) state)->error ;
	return SRC_ERR_NO_ERROR ;
} /* src_error */

const char*
src_strerror (int error)
{
	switch (error)
	{	case SRC_ERR_NO_ERROR :
				return "No error" ;
		case SRC_ERR_MALLOC_FAILED :
				return "Malloc failed." ;
		case SRC_ERR_BAD_STATE :
				return "SRC_STATE pointer is NULL." ;
		case SRC_ERR_BAD_DATA :
				return "SRC_DATA pointer is NULL." ;
		case SRC_ERR_BAD_DATA_PTR :
				return "SRC_DATA->data_out is NULL." ;
		case SRC_ERR_NO_PRIVATE :
				return "Internal error. No private data." ;
		case SRC_ERR_BAD_SRC_RATIO :
				return "SRC ratio outside [-12, 12] range." ;
		case SRC_ERR_BAD_SINC_STATE :
				return "src_process() called without reset after end_of_input." ;
		case SRC_ERR_BAD_PROC_PTR :
				return "Internal error. No process pointer." ;
		case SRC_ERR_SHIFT_BITS :
				return "Internal error. SHIFT_BITS too large." ;
		case SRC_ERR_FILTER_LEN :
				return "Internal error. Filter length too large." ;
		case SRC_ERR_BAD_CONVERTER :
				return "Bad converter number." ;
		case SRC_ERR_BAD_CHANNEL_COUNT :
				return "Channel count must be >= 1." ;
		case SRC_ERR_SINC_BAD_BUFFER_LEN :
				return "Internal error. Bad buffer length. Please report this." ;
		case SRC_ERR_SIZE_INCOMPATIBILITY :
				return "Internal error. Input data / internal buffer size difference. Please report this." ;
		case SRC_ERR_BAD_PRIV_PTR :
				return "Internal error. Private pointer is NULL. Please report this." ;

		case SRC_ERR_DATA_OVERLAP :
				return "Input and output data arrays overlap." ;

		case SRC_ERR_MAX_ERROR :
				return "Placeholder. No error defined for this error number." ;

		default : 						break ;
		}

	return NULL ;
} /* src_strerror */

/*==============================================================================
**	Simple interface for performing a single conversion from input buffer to
** output buffer at a fixed conversion ratio.
*/

int
src_simple (SRC_DATA *src_data, int converter, int channels)
{	SRC_STATE	*src_state ;
	int 		error ;

	if ((src_state = src_new (converter, channels, &error)) == NULL)
		return error ;

	src_data->end_of_input = 1 ; /* Only one buffer worth of input. */

	error = src_process (src_state, src_data) ;

	src_state = src_delete (src_state) ;

	return error ;
} /* src_simple */

