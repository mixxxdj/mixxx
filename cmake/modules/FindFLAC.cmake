# This file is part of Mixxx, Digital DJ'ing software.
# Copyright (C) 2001-2022 Mixxx Development Team
# Distributed under the GNU General Public Licence (GPL) version 2 or any later
# later version. See the LICENSE file for details.

#[=======================================================================[.rst:
FindFLAC
--------

Finds the FLAC library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``FLAC::FLAC``
  The FLAC library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``FLAC_FOUND``
  True if the system has the FLAC library.
``FLAC_INCLUDE_DIRS``
  Include directories needed to use FLAC.
``FLAC_LIBRARIES``
  Libraries needed to link to FLAC.
``FLAC_DEFINITIONS``
  Compile definitions needed to use FLAC.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``FLAC_INCLUDE_DIR``
  The directory containing ``FLAC/all.h``.
``FLAC_LIBRARY``
  The path to the FLAC library.

#]=======================================================================]

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_FLAC QUIET flac)
endif()

find_path(FLAC_INCLUDE_DIR
  NAMES FLAC/all.h
  PATHS ${PC_FLAC_INCLUDE_DIRS}
  DOC "FLAC include directory")
mark_as_advanced(FLAC_INCLUDE_DIR)

find_library(FLAC_LIBRARY
  NAMES FLAC
  PATHS ${PC_FLAC_LIBRARY_DIRS}
  DOC "FLAC library"
)
mark_as_advanced(FLAC_LIBRARY)

if(DEFINED PC_FLAC_VERSION AND NOT PC_FLAC_VERSION STREQUAL "")
  set(FLAC_VERSION "${PC_FLAC_VERSION}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  FLAC
  REQUIRED_VARS FLAC_LIBRARY FLAC_INCLUDE_DIR
  VERSION_VAR FLAC_VERSION
)

if(FLAC_FOUND)
  set(FLAC_LIBRARIES "${FLAC_LIBRARY}")
  set(FLAC_INCLUDE_DIRS "${FLAC_INCLUDE_DIR}")
  set(FLAC_DEFINITIONS ${PC_FLAC_CFLAGS_OTHER})

  if(NOT TARGET FLAC::FLAC)
    add_library(FLAC::FLAC UNKNOWN IMPORTED)
    set_target_properties(FLAC::FLAC
      PROPERTIES
        IMPORTED_LOCATION "${FLAC_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_FLAC_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${FLAC_INCLUDE_DIR}"
    )
  endif()
endif()
