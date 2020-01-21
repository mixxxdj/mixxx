# This file is part of Mixxx, Digital DJ'ing software.
# Copyright (C) 2001-2020 Mixxx Development Team
# Distributed under the GNU General Public Licence (GPL) version 2 or any later
# later version. See the LICENSE file for details.

#[=======================================================================[.rst:
FindOggVorbis
---------------

Finds the OggVorbis library.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``OggVorbis_FOUND``
  True if the system has the OggVorbis library.
``OggVorbis_INCLUDE_DIRS``
  Include directories needed to use OggVorbis.
``OggVorbis_LIBRARIES``
  Libraries needed to link to OggVorbis.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``Ogg_INCLUDE_DIR``
  The directory containing ``ogg/ogg.h``.
``Ogg_LIBRARY``
  The path to the OggVorbis library.
``Vorbis_INCLUDE_DIR``
  The directory containing ``vorbis/vorbisfile.h``.
``Vorbis_LIBRARY``
  The path to the vorbis library.
``VorbisFile_LIBRARY``
  The path to the vorbisfile library.
``VorbisEnc_LIBRARY``
  The path to the vorbisenc library.
``Vorbis_LIBRARIES``
  Libraries needed to link to vorbis.

#]=======================================================================]
find_path(Ogg_INCLUDE_DIR NAMES ogg/ogg.h DOC "Ogg include directory")
mark_as_advanced(Ogg_INCLUDE_DIR)

find_library(Ogg_LIBRARY NAMES ogg DOC "Ogg library")
mark_as_advanced(Ogg_LIBRARY)

find_path(Vorbis_INCLUDE_DIR
  NAMES vorbis/vorbisfile.h
  DOC "Vorbis include directory"
)
mark_as_advanced(Vorbis_INCLUDE_DIR)

find_library(Vorbis_LIBRARY NAMES vorbis DOC "Vorbis library")
mark_as_advanced(Vorbis_LIBRARY)

find_library(VorbisFile_LIBRARY NAMES vorbisfile DOC "Vorbisfile library")
mark_as_advanced(VorbisFile_LIBRARY)

if(NOT MSVC)
  find_library(VorbisEnc_LIBRARY NAMES vorbisenc DOC "Vorbisenc library")
  mark_as_advanced(VorbisEnc_LIBRARY)
  set(Vorbis_LIBRARIES
    ${VorbisEnc_LIBRARY}
    ${VorbisFile_LIBRARY}
    ${Vorbis_LIBRARY}
  )
else()
  set(Vorbis_LIBRARIES ${VorbisFile_LIBRARY} ${Vorbis_LIBRARY})
endif()
mark_as_advanced(Vorbis_LIBRARIES)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  OggVorbis
  REQUIRED_VARS
  Ogg_INCLUDE_DIR
  Vorbis_INCLUDE_DIR
  Ogg_LIBRARY
  Vorbis_LIBRARIES
)

if(OggVorbis_FOUND)
  set(OggVorbis_LIBRARIES ${Ogg_LIBRARY} ${Vorbis_LIBRARIES})
  set(OggVorbis_INCLUDE_DIRS ${Ogg_INCLUDE_DIR} ${Vorbis_INCLUDE_DIR})
endif()
