/*
	Audio File Library
	Copyright (C) 1999, Elliot Lee <sopwith@redhat.com>

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
	af_vfs.h

	Virtual file operations for the Audio File Library.
*/

#ifndef AUDIOFILE_VFS_H
#define AUDIOFILE_VFS_H 1

#include <stdio.h>

struct _AFvirtualfile
{
  ssize_t (*read) (AFvirtualfile *vfile, void *data, size_t nbytes);
  long (*length) (AFvirtualfile *vfile);
  ssize_t (*write) (AFvirtualfile *vfile, const void *data, size_t nbytes);
  void (*destroy)(AFvirtualfile *vfile);
  long (*seek)   (AFvirtualfile *vfile, long offset, int is_relative);
  long (*tell)   (AFvirtualfile *vfile);

  void *closure;
};

AFvirtualfile *af_virtual_file_new (void);
AFvirtualfile *af_virtual_file_new_for_file (FILE *fh);
void af_virtual_file_destroy (AFvirtualfile *vfile);

size_t af_fread (void *data, size_t size, size_t nmemb, AFvirtualfile *vfile);
size_t af_fwrite (const void *data, size_t size, size_t nmemb, AFvirtualfile *vfile);
int af_fclose (AFvirtualfile *vfile);
long af_flength (AFvirtualfile *vfile);
int af_fseek (AFvirtualfile *vfile, long offset, int whence);
long af_ftell (AFvirtualfile *vfile);

#endif
