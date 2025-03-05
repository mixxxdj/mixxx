# This file is part of Mixxx, Digital DJ'ing software.
# Copyright (C) 2001-2025 Mixxx Development Team
# Distributed under the GNU General Public Licence (GPL) version 2 or any later
# later version. See the LICENSE file for details.

#[=======================================================================[.rst:
Findsord
--------

Finds the sord library and its development headers.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``sord::sord``
  The sord library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``sord_FOUND``
  True if the system has the sord library.
``sord_INCLUDE_DIRS``
  Include directories needed to use sord.
``sord_LIBRARIES``
  Libraries needed to link to sord.
``sord_DEFINITIONS``
  Compile definitions needed to use sord.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``sord_INCLUDE_DIR``
  The directory containing ``sord/sord.h``.
``sord_LIBRARY``
  The path to the sord library.

#]=======================================================================]

include(IsStaticLibrary)

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_sord QUIET sord-0)
endif()

find_path(
  sord_INCLUDE_DIR
  NAMES sord/sord.h
  HINTS ${PC_sord_INCLUDE_DIRS}
  DOC "sord include directory"
)
mark_as_advanced(sord_INCLUDE_DIR)

find_library(
  sord_LIBRARY
  NAMES sord-0
  HINTS ${PC_sord_LIBRARY_DIRS}
  DOC "sord library"
)
mark_as_advanced(sord_LIBRARY)

if(DEFINED PC_sord_VERSION AND NOT PC_sord_VERSION STREQUAL "")
  set(sord_VERSION "${PC_sord_VERSION}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  sord
  REQUIRED_VARS sord_LIBRARY sord_INCLUDE_DIR
  VERSION_VAR sord_VERSION
)

if(sord_FOUND)
  set(sord_LIBRARIES "${sord_LIBRARY}")
  set(sord_INCLUDE_DIRS "${sord_INCLUDE_DIR}")
  set(sord_DEFINITIONS ${PC_sord_CFLAGS_OTHER})

  if(NOT TARGET sord::sord)
    add_library(sord::sord UNKNOWN IMPORTED)
    set_target_properties(
      sord::sord
      PROPERTIES
        IMPORTED_LOCATION "${sord_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_sord_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${sord_INCLUDE_DIR}"
    )
  endif()
endif()