/*
 * libid3tag - ID3 tag manipulation library
 * Copyright (C) 2000-2001 Robert Leslie
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: id3tag.h 363 2003-06-10 15:29:38Z tuehaste $
 */

# ifndef LIBID3TAG_ID3TAG_H
# define LIBID3TAG_ID3TAG_H

# ifdef __cplusplus
extern "C" {
# endif

# define ID3_TAG_VERSION		0x0400
# define ID3_TAG_VERSION_MAJOR(x)	(((x) >> 8) & 0xff)
# define ID3_TAG_VERSION_MINOR(x)	(((x) >> 0) & 0xff)

struct id3_tag {
  unsigned int refcount;
  unsigned int version;
  int flags;
  int extendedflags;
  int restrictions;
  int options;
  unsigned int nframes;
  struct id3_frame **frames;
  unsigned long paddedsize;
};

# define ID3_TAG_QUERYSIZE	10

/* ID3v1 field frames */

# define ID3_FRAME_TITLE	"TIT2"
# define ID3_FRAME_ARTIST	"TPE1"
# define ID3_FRAME_ALBUM	"TALB"
# define ID3_FRAME_YEAR		"TDRC"
# define ID3_FRAME_TRACK	"TRCK"
# define ID3_FRAME_GENRE	"TCON"
# define ID3_FRAME_COMMENT	"COMM"

/* special frames */

# define ID3_FRAME_OBSOLETE	"ZOBS"

/* tag flags */

enum {
  ID3_TAG_FLAG_UNSYNCHRONISATION     = 0x80,
  ID3_TAG_FLAG_EXTENDEDHEADER        = 0x40,
  ID3_TAG_FLAG_EXPERIMENTALINDICATOR = 0x20,
  ID3_TAG_FLAG_FOOTERPRESENT         = 0x10,

  ID3_TAG_FLAG_KNOWNFLAGS            = 0xf0
};

/* tag extended flags */

enum {
  ID3_TAG_EXTENDEDFLAG_TAGISANUPDATE   = 0x40,
  ID3_TAG_EXTENDEDFLAG_CRCDATAPRESENT  = 0x20,
  ID3_TAG_EXTENDEDFLAG_TAGRESTRICTIONS = 0x10,

  ID3_TAG_EXTENDEDFLAG_KNOWNFLAGS      = 0x70
};

/* tag restrictions */

enum {
  ID3_TAG_RESTRICTION_TAGSIZE_MASK             = 0xc0,
  ID3_TAG_RESTRICTION_TAGSIZE_128_FRAMES_1_MB  = 0x00,
  ID3_TAG_RESTRICTION_TAGSIZE_64_FRAMES_128_KB = 0x40,
  ID3_TAG_RESTRICTION_TAGSIZE_32_FRAMES_40_KB  = 0x80,
  ID3_TAG_RESTRICTION_TAGSIZE_32_FRAMES_4_KB   = 0xc0
};

enum {
  ID3_TAG_RESTRICTION_TEXTENCODING_MASK        = 0x20,
  ID3_TAG_RESTRICTION_TEXTENCODING_NONE        = 0x00,
  ID3_TAG_RESTRICTION_TEXTENCODING_LATIN1_UTF8 = 0x20
};

enum {
  ID3_TAG_RESTRICTION_TEXTSIZE_MASK            = 0x18,
  ID3_TAG_RESTRICTION_TEXTSIZE_NONE            = 0x00,
  ID3_TAG_RESTRICTION_TEXTSIZE_1024_CHARS      = 0x08,
  ID3_TAG_RESTRICTION_TEXTSIZE_128_CHARS       = 0x10,
  ID3_TAG_RESTRICTION_TEXTSIZE_30_CHARS        = 0x18
};

enum {
  ID3_TAG_RESTRICTION_IMAGEENCODING_MASK       = 0x04,
  ID3_TAG_RESTRICTION_IMAGEENCODING_NONE       = 0x00,
  ID3_TAG_RESTRICTION_IMAGEENCODING_PNG_JPEG   = 0x04
};

enum {
  ID3_TAG_RESTRICTION_IMAGESIZE_MASK           = 0x03,
  ID3_TAG_RESTRICTION_IMAGESIZE_NONE           = 0x00,
  ID3_TAG_RESTRICTION_IMAGESIZE_256_256        = 0x01,
  ID3_TAG_RESTRICTION_IMAGESIZE_64_64          = 0x02,
  ID3_TAG_RESTRICTION_IMAGESIZE_64_64_EXACT    = 0x03
};

/* library options */

enum {
  ID3_TAG_OPTION_UNSYNCHRONISATION = 0x0001,
  ID3_TAG_OPTION_COMPRESSION       = 0x0002,
  ID3_TAG_OPTION_CRC               = 0x0004,

  ID3_TAG_OPTION_APPENDEDTAG       = 0x0010,
  ID3_TAG_OPTION_FILEALTERED       = 0x0020,

  ID3_TAG_OPTION_ID3V1             = 0x0100
};

typedef unsigned char id3_byte_t;
typedef unsigned long id3_length_t;

typedef unsigned long id3_ucs4_t;

typedef unsigned char id3_latin1_t;
typedef unsigned short id3_utf16_t;
typedef signed char id3_utf8_t;

struct id3_frame {
  char id[5];
  char const *description;
  unsigned int refcount;
  int flags;
  int group_id;
  int encryption_method;
  id3_byte_t *encoded;
  id3_length_t encoded_length;
  id3_length_t decoded_length;
  unsigned int nfields;
  union id3_field *fields;
};

enum {
  /* frame status flags */
  ID3_FRAME_FLAG_TAGALTERPRESERVATION	= 0x4000,
  ID3_FRAME_FLAG_FILEALTERPRESERVATION	= 0x2000,
  ID3_FRAME_FLAG_READONLY		= 0x1000,

  ID3_FRAME_FLAG_STATUSFLAGS            = 0xff00,

  /* frame format flags */
  ID3_FRAME_FLAG_GROUPINGIDENTITY	= 0x0040,
  ID3_FRAME_FLAG_COMPRESSION		= 0x0008,
  ID3_FRAME_FLAG_ENCRYPTION		= 0x0004,
  ID3_FRAME_FLAG_UNSYNCHRONISATION	= 0x0002,
  ID3_FRAME_FLAG_DATALENGTHINDICATOR	= 0x0001,

  ID3_FRAME_FLAG_FORMATFLAGS            = 0x00ff,

  ID3_FRAME_FLAG_KNOWNFLAGS             = 0x704f
};

enum id3_field_type {
  ID3_FIELD_TYPE_TEXTENCODING,
  ID3_FIELD_TYPE_LATIN1,
  ID3_FIELD_TYPE_LATIN1FULL,
  ID3_FIELD_TYPE_LATIN1LIST,
  ID3_FIELD_TYPE_STRING,
  ID3_FIELD_TYPE_STRINGFULL,
  ID3_FIELD_TYPE_STRINGLIST,
  ID3_FIELD_TYPE_LANGUAGE,
  ID3_FIELD_TYPE_FRAMEID,
  ID3_FIELD_TYPE_DATE,
  ID3_FIELD_TYPE_INT8,
  ID3_FIELD_TYPE_INT16,
  ID3_FIELD_TYPE_INT24,
  ID3_FIELD_TYPE_INT32,
  ID3_FIELD_TYPE_INT32PLUS,
  ID3_FIELD_TYPE_BINARYDATA
};

enum id3_field_textencoding {
  ID3_FIELD_TEXTENCODING_ISO_8859_1 = 0x00,
  ID3_FIELD_TEXTENCODING_UTF_16     = 0x01,
  ID3_FIELD_TEXTENCODING_UTF_16BE   = 0x02,
  ID3_FIELD_TEXTENCODING_UTF_8      = 0x03
};

union id3_field {
  enum id3_field_type type;
  struct {
    enum id3_field_type type;
    signed long value;
  } number;
  struct {
    enum id3_field_type type;
    id3_latin1_t *ptr;
  } latin1;
  struct {
    enum id3_field_type type;
    unsigned int nstrings;
    id3_latin1_t **strings;
  } latin1list;
  struct {
    enum id3_field_type type;
    id3_ucs4_t *ptr;
  } string;
  struct {
    enum id3_field_type type;
    unsigned int nstrings;
    id3_ucs4_t **strings;
  } stringlist;
  struct {
    enum id3_field_type type;
    char value[9];
  } immediate;
  struct {
    enum id3_field_type type;
    id3_byte_t *data;
    id3_length_t length;
  } binary;
};

/* tag interface */

struct id3_tag *id3_tag_new(void);
void id3_tag_delete(struct id3_tag *);

void id3_tag_clearframes(struct id3_tag *);

int id3_tag_attachframe(struct id3_tag *, struct id3_frame *);
int id3_tag_detachframe(struct id3_tag *, struct id3_frame *);

struct id3_frame *id3_tag_findframe(struct id3_tag const *,
				    char const *, unsigned int);

signed long id3_tag_query(id3_byte_t const *, id3_length_t);

struct id3_tag *id3_tag_parse(id3_byte_t const *, id3_length_t);
id3_length_t id3_tag_render(struct id3_tag const *, id3_byte_t *);

/* file interface */

enum id3_file_mode {
  ID3_FILE_MODE_READONLY = 0,
  ID3_FILE_MODE_READWRITE
};

struct id3_file *id3_file_open(char const *, enum id3_file_mode);
struct id3_file *id3_file_fdopen(int, enum id3_file_mode);
void id3_file_close(struct id3_file *);

struct id3_tag *id3_file_tag(struct id3_file const *);
int id3_file_update(struct id3_file *);

/* ucs4 interface */

id3_latin1_t *id3_ucs4_latin1duplicate(id3_ucs4_t const *);
id3_utf16_t *id3_ucs4_utf16duplicate(id3_ucs4_t const *);
id3_utf8_t *id3_ucs4_utf8duplicate(id3_ucs4_t const *);

void id3_ucs4_putnumber(id3_ucs4_t *, unsigned long);
unsigned long id3_ucs4_getnumber(id3_ucs4_t const *);

/* genre interface */

id3_ucs4_t const *id3_genre_name(id3_ucs4_t const *);

/* field interface */

int id3_field_setint(union id3_field *, signed long);
int id3_field_settextencoding(union id3_field *, enum id3_field_textencoding);
int id3_field_setstrings(union id3_field *, unsigned int, id3_ucs4_t **);
int id3_field_addstring(union id3_field *, id3_ucs4_t const *);
int id3_field_setlanguage(union id3_field *, char const *);
int id3_field_setlatin1(union id3_field *, id3_latin1_t const *);
int id3_field_setfulllatin1(union id3_field *, id3_latin1_t const *);
int id3_field_setstring(union id3_field *, id3_ucs4_t const *);
int id3_field_setfullstring(union id3_field *, id3_ucs4_t const *);
int id3_field_setframeid(union id3_field *, char const *);
int id3_field_setbinarydata(union id3_field *,
			    id3_byte_t const *, id3_length_t);

signed long id3_field_getint(union id3_field const *);
id3_ucs4_t const *id3_field_getstring(union id3_field const *);
id3_ucs4_t const *id3_field_getfullstring(union id3_field const *);
unsigned int id3_field_getnstrings(union id3_field const *);
id3_ucs4_t const *id3_field_getstrings(union id3_field const *,
				       unsigned int index);
char const *id3_field_getframeid(union id3_field const *);
id3_byte_t const *id3_field_getbinarydata(union id3_field const *,
					  id3_length_t *);

/* version interface */

# define ID3_VERSION_MAJOR	0
# define ID3_VERSION_MINOR	14
# define ID3_VERSION_PATCH	2
# define ID3_VERSION_EXTRA	" (beta)"

# define ID3_VERSION_STRINGIZE(str)	#str
# define ID3_VERSION_STRING(num)	ID3_VERSION_STRINGIZE(num)

# define ID3_VERSION	ID3_VERSION_STRING(ID3_VERSION_MAJOR) "."  \
			ID3_VERSION_STRING(ID3_VERSION_MINOR) "."  \
			ID3_VERSION_STRING(ID3_VERSION_PATCH)  \
			ID3_VERSION_EXTRA

extern char const id3_version[];
extern char const id3_copyright[];
extern char const id3_author[];
extern char const id3_build[];

# ifdef __cplusplus
}
# endif

# endif
