# This file is part of Mixxx, Digital DJ'ing software.
# Copyright (C) 2001-2020 Mixxx Development Team
# Distributed under the GNU General Public Licence (GPL) version 2 or any later
# later version. See the LICENSE file for details.

#[=======================================================================[.rst:
FindOpus
--------

Finds the Opus library.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``Opus_FOUND``
  True if the system has the Opus library.
``Opus_INCLUDE_DIRS``
  Include directories needed to use Opus.
``Opus_LIBRARIES``
  Libraries needed to link to Opus.
``Opus_DEFINITIONS``
  Compile definitions needed to use Opus.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``Opus_INCLUDE_DIR``
  The directory containing ``opus.h``.
``Opus_LIBRARY``
  The path to the Opus library.
``OpusFile_INCLUDE_DIR``
  The directory containing ``opusfile.h``.
``OpusFile_LIBRARY``
  The path to the Opus library.

#]=======================================================================]

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_Opus QUIET opus)
  pkg_check_modules(PC_OpusFile QUIET opusfile)
endif()

find_path(Opus_INCLUDE_DIR
  NAMES opus/opus.h
  PATHS ${PC_Opus_INCLUDE_DIRS}
  DOC "Opus include directory")
mark_as_advanced(Opus_INCLUDE_DIR)

find_library(Opus_LIBRARY
  NAMES opus
  PATHS ${PC_Opus_LIBRARY_DIRS}
  DOC "Opus library"
)
mark_as_advanced(Opus_LIBRARY)

find_path(OpusFile_INCLUDE_DIR
  NAMES opusfile.h
  PATH_SUFFIXES opus
  PATHS ${PC_OpusFile_INCLUDE_DIRS}
  DOC "Opusfile include directory")
mark_as_advanced(OpusFile_INCLUDE_DIR)

find_library(OpusFile_LIBRARY
  NAMES opusfile
  PATHS ${PC_OpusFile_LIBRARY_DIRS}
  DOC "Opusfile library"
)
mark_as_advanced(OpusFile_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Opus
  DEFAULT_MSG
  Opus_LIBRARY
  Opus_INCLUDE_DIR
  OpusFile_LIBRARY
  OpusFile_INCLUDE_DIR
)

if(Opus_FOUND)
  set(Opus_LIBRARIES ${Opus_LIBRARY} ${OpusFile_LIBRARY})
  set(Opus_INCLUDE_DIRS ${Opus_INCLUDE_DIR} ${OpusFile_INCLUDE_DIR})
  set(Opus_DEFINITIONS ${PC_Opus_CFLAGS_OTHER} ${PC_OpusFile_CFLAGS_OTHER})
endif()
