# This file is part of Mixxx, Digital DJ'ing software.
# Copyright (C) 2001-2020 Mixxx Development Team
# Distributed under the GNU General Public Licence (GPL) version 2 or any later
# later version. See the LICENSE file for details.

#[=======================================================================[.rst:
FindVorbis
----------

Finds the Vorbis library.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``Vorbis_FOUND``
  True if the system has the OggVorbis library.
``Vorbis_INCLUDE_DIRS``
  Include directories needed to use OggVorbis.
``Vorbis_LIBRARIES``
  Libraries needed to link to OggVorbis.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``Vorbis_vorbis_INCLUDE_DIR``
  The directory containing ``vorbis/vorbis.h``.
``Vorbis_vorbisenc_INCLUDE_DIR``
  The directory containing ``vorbis/vorbisenc.h``.
``Vorbis_vorbisfile_INCLUDE_DIR``
  The directory containing ``vorbis/vorbisfile.h``.
``Vorbis_vorbis_LIBRARY``
  The path to the vorbis library.
``Vorbis_vorbisenc_LIBRARY``
  The path to the vorbisenc library.
``Vorbis_vorbisfile_LIBRARY``
  The path to the vorbisfile library.
``Vorbis_LIBRARIES``
  Libraries needed to link to vorbis.

#]=======================================================================]

find_path(Vorbis_vorbis_INCLUDE_DIR
  NAMES vorbis/codec.h
  DOC "Vorbis include directory"
)
mark_as_advanced(Vorbis_vorbis_INCLUDE_DIR)

find_path(Vorbis_vorbisenc_INCLUDE_DIR
  NAMES vorbis/vorbisenc.h
  DOC "Vorbisenc include directory"
)
mark_as_advanced(Vorbis_vorbisenc_INCLUDE_DIR)

find_path(Vorbis_vorbisfile_INCLUDE_DIR
  NAMES vorbis/vorbisfile.h
  DOC "Vorbisfile include directory"
)
mark_as_advanced(Vorbis_vorbisfile_INCLUDE_DIR)

find_library(Vorbis_vorbis_LIBRARY
  NAMES vorbis
  DOC "Vorbis library")
mark_as_advanced(Vorbis_vorbis_LIBRARY)

find_library(Vorbis_vorbisenc_LIBRARY
  NAMES vorbisenc
  DOC "Vorbisenc library")
mark_as_advanced(Vorbis_vorbisenc_LIBRARY)

find_library(Vorbis_vorbisfile_LIBRARY
  NAMES vorbisfile
  DOC "Vorbisfile library")
mark_as_advanced(Vorbis_vorbisfile_LIBRARY)

if(NOT Vorbis_FIND_COMPONENTS)
  set(Vorbis_FIND_COMPONENTS "vorbis;vorbisenc;vorbisfile")
endif()

foreach(component ${Vorbis_FIND_COMPONENTS})
  if(Vorbis_${component}_INCLUDE_DIR AND Vorbis_${component}_LIBRARY)
    set(Vorbis_${component}_FOUND TRUE)
    if(NOT TARGET Vorbis::${component})
      add_library(Vorbis::${component} UNKNOWN IMPORTED)
      set_target_properties(Vorbis::${component}
        PROPERTIES
          IMPORTED_LOCATION "${Vorbis_${component}_LIBRARY}"
          INTERFACE_INCLUDE_DIRECTORIES "${Vorbis_${component}_INCLUDE_DIR}"
      )
    endif()
  else()
    set(Vorbis_${component}_FOUND FALSE)
  endif()
endforeach()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Vorbis
    REQUIRED_VARS Vorbis_vorbis_INCLUDE_DIR Vorbis_vorbis_LIBRARY
    HANDLE_COMPONENTS
)
