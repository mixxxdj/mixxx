# This file is part of Mixxx, Digital DJ'ing software.
# Copyright (C) 2001-2024 Mixxx Development Team
# Distributed under the GNU General Public Licence (GPL) version 2 or any later
# later version. See the LICENSE file for details.

#[=======================================================================[.rst:
Findlo
--------

Finds the lo library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``lo::lo``
  The lo library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``lo_FOUND``
  True if the system has the lo library.
``lo_INCLUDE_DIRS``
  Include directories needed to use lo.
``lo_LIBRARIES``
  Libraries needed to link to lo.
``lo_DEFINITIONS``
  Compile definitions needed to use lo.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``lo_INCLUDE_DIR``
  The directory containing ``lo/lo.h``.
``lo_LIBRARY``
  The path to the lo library.

#]=======================================================================]

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_lo QUIET lo)
endif()

find_path(
  lo_INCLUDE_DIR
  NAMES lo/lo.h
  HINTS ${PC_lo_INCLUDE_DIRS}
  ENV CPATH
  ENV CMAKE_INCLUDE_PATH
  PATH_SUFFIXES include
  DOC "lo include directory"
)
mark_as_advanced(lo_INCLUDE_DIR)

find_library(
  lo_LIBRARY
  NAMES lo
  HINTS ${PC_lo_LIBRARY_DIRS}
  ENV LIBRARY_PATH
  ENV CMAKE_LIBRARY_PATH
  PATH_SUFFIXES lib
  DOC "lo library"
)
mark_as_advanced(lo_LIBRARY)

if(DEFINED PC_lo_VERSION AND NOT PC_lo_VERSION STREQUAL "")
  set(lo_VERSION "${PC_lo_VERSION}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  lo
  REQUIRED_VARS lo_LIBRARY lo_INCLUDE_DIR
  VERSION_VAR lo_VERSION
)

if(lo_FOUND)
  set(lo_LIBRARIES "${lo_LIBRARY}")
  set(lo_INCLUDE_DIRS "${lo_INCLUDE_DIR}")
  if(NOT TARGET lo::lo)
    add_library(lo::lo UNKNOWN IMPORTED)
    set_target_properties(
      lo::lo
      PROPERTIES
        IMPORTED_LOCATION "${lo_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_lo_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${lo_INCLUDE_DIR}"
    )
  endif()
endif()
