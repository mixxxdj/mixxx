# This file is part of Mixxx, Digital DJ'ing software.
# Copyright (C) 2001-2020 Mixxx Development Team
# Distributed under the GNU General Public Licence (GPL) version 2 or any later
# later version. See the LICENSE file for details.

#[=======================================================================[.rst:
FindSndFile
-----------

Finds the SndFile library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``SndFile::SndFile``
  The SndFile library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``SndFile_FOUND``
  True if the system has the SndFile library.
``SndFile_INCLUDE_DIRS``
  Include directories needed to use SndFile.
``SndFile_LIBRARIES``
  Libraries needed to link to SndFile.
``SndFile_DEFINITIONS``
  Compile definitions needed to use SndFile.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``SndFile_INCLUDE_DIR``
  The directory containing ``sndfile.h``.
``SndFile_LIBRARY``
  The path to the SndFile library.

#]=======================================================================]

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_SndFile QUIET sndfile)
endif()

find_path(SndFile_INCLUDE_DIR
  NAMES sndfile.h
  PATHS ${PC_SndFile_INCLUDE_DIRS}
  PATH_SUFFIXES sndfile
  DOC "SndFile include directory")
mark_as_advanced(SndFile_INCLUDE_DIR)

find_library(SndFile_LIBRARY
  NAMES sndfile sndfile-1
  PATHS ${PC_SndFile_LIBRARY_DIRS}
  DOC "SndFile library"
)
mark_as_advanced(SndFile_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  SndFile
  DEFAULT_MSG
  SndFile_LIBRARY
  SndFile_INCLUDE_DIR
)

file(STRINGS "${SndFile_INCLUDE_DIR}/sndfile.h" SndFile_SUPPORTS_SET_COMPRESSION_LEVEL REGEX ".*SFC_SET_COMPRESSION_LEVEL.*")
if(SndFile_SUPPORTS_SET_COMPRESSION_LEVEL)
  set(SndFile_SUPPORTS_SET_COMPRESSION_LEVEL ON)
else()
  set(SndFile_SUPPORTS_SET_COMPRESSION_LEVEL OFF)
endif()
mark_as_advanced(SndFile_SUPPORTS_SET_COMPRESSION_LEVEL)

if(SndFile_FOUND)
  set(SndFile_LIBRARIES "${SndFile_LIBRARY}")
  set(SndFile_INCLUDE_DIRS "${SndFile_INCLUDE_DIR}")
  set(SndFile_DEFINITIONS ${PC_SndFile_CFLAGS_OTHER})

  if(NOT TARGET SndFile::sndfile)
    add_library(SndFile::sndfile UNKNOWN IMPORTED)
    set_target_properties(SndFile::sndfile
      PROPERTIES
        IMPORTED_LOCATION "${SndFile_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_SndFile_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${SndFile_INCLUDE_DIR}"
    )
  endif()
endif()
