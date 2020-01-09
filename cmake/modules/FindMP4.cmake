# This file is part of Mixxx, Digital DJ'ing software.
# Copyright (C) 2001-2020 Mixxx Development Team
# Distributed under the GNU General Public Licence (GPL) version 2 or any later
# later version. See the LICENSE file for details.

#[=======================================================================[.rst:
FindMP4
-------

Finds the MP4 library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``MP4::MP4``
  The MP4 library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``MP4_FOUND``
  True if the system has the MP4 library.
``MP4_INCLUDE_DIRS``
  Include directories needed to use MP4.
``MP4_LIBRARIES``
  Libraries needed to link to MP4.
``MP4_DEFINITIONS``
  Compile definitions needed to use MP4.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``MP4_INCLUDE_DIR``
  The directory containing ``mp4/mp4.h``.
``MP4_LIBRARY``
  The path to the MP4 library.

#]=======================================================================]

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_MP4 QUIET mp4)
endif()

find_path(MP4_INCLUDE_DIR
  NAMES mp4/mp4.h
  PATHS ${PC_MP4_INCLUDE_DIRS}
  DOC "MP4 include directory")
mark_as_advanced(MP4_INCLUDE_DIR)

find_library(MP4_LIBRARY
  NAMES mp4
  PATHS ${PC_MP4_LIBRARY_DIRS}
  DOC "MP4 library"
)
mark_as_advanced(MP4_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  MP4
  DEFAULT_MSG
  MP4_LIBRARY
  MP4_INCLUDE_DIR
)

if(MP4_FOUND)
  set(MP4_LIBRARIES "${MP4_LIBRARY}")
  set(MP4_INCLUDE_DIRS "${MP4_INCLUDE_DIR}")
  set(MP4_DEFINITIONS ${PC_MP4_CFLAGS_OTHER})

  if(NOT TARGET MP4::MP4)
    add_library(MP4::MP4 UNKNOWN IMPORTED)
    set_target_properties(MP4::MP4
      PROPERTIES
        IMPORTED_LOCATION "${MP4_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_MP4_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${MP4_INCLUDE_DIR}"
    )
  endif()
endif()
