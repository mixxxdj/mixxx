/*
	Audio File Library
	Copyright (C) 1998-2000, Michael Pruett <michael@68k.org>

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Library General Public
	License as published by the Free Software Foundation; either
	version 2 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Library General Public License for more details.

	You should have received a copy of the GNU Library General Public
	License along with this library; if not, write to the
	Free Software Foundation, Inc., 59 Temple Place - Suite 330,
	Boston, MA  02111-1307  USA.
*/

/*
	aupvlist.h

	This file contains the interface to the parameter value list data
	structures and routines.
*/

#ifndef AUPVLIST_H
#define AUPVLIST_H

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

enum
{
	AU_PVTYPE_LONG = 1,
	AU_PVTYPE_DOUBLE = 2,
	AU_PVTYPE_PTR = 3
};

typedef struct _AUpvlist *AUpvlist;

#define AU_NULL_PVLIST ((struct _AUpvlist *) 0)

AUpvlist AUpvnew (int maxItems);
int AUpvgetmaxitems (AUpvlist);
int AUpvfree (AUpvlist);
int AUpvsetparam (AUpvlist, int item, int param);
int AUpvsetvaltype (AUpvlist, int item, int type);
int AUpvsetval (AUpvlist, int item, void *val);
int AUpvgetparam (AUpvlist, int item, int *param);
int AUpvgetvaltype (AUpvlist, int item, int *type);
int AUpvgetval (AUpvlist, int item, void *val);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AUPVLIST_H */
