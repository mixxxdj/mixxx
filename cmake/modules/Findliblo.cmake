# This file is part of Mixxx, Digital DJ'ing software.
# Copyright (C) 2001-2024 Mixxx Development Team
# Distributed under the GNU General Public Licence (GPL) version 2 or any later
# later version. See the LICENSE file for details.

#[=======================================================================[.rst:
Findliblo
--------

Finds the liblo library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``liblo::liblo``
  The liblo library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``liblo_FOUND``
  True if the system has the liblo library.
``liblo_INCLUDE_DIRS``
  Include directories needed to use liblo.
``liblo_LIBRARIES``
  Libraries needed to link to liblo.
``liblo_DEFINITIONS``
  Compile definitions needed to use liblo.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``liblo_INCLUDE_DIR``
  The directory containing ``liblo/liblo.h``.
``liblo_LIBRARY``
  The path to the liblo library.

#]=======================================================================]

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_liblo QUIET liblo)
endif()

find_path(
  liblo_INCLUDE_DIR
  NAMES lo/lo.h
  HINTS ${PC_liblo_INCLUDE_DIRS}
  DOC "liblo include directory"
)
mark_as_advanced(liblo_INCLUDE_DIR)

find_library(
  liblo_LIBRARY
  NAMES lo
  HINTS ${PC_liblo_LIBRARY_DIRS}
  DOC "liblo library"
)
mark_as_advanced(liblo_LIBRARY)

if(DEFINED PC_liblo_VERSION AND NOT PC_liblo_VERSION STREQUAL "")
  set(liblo_VERSION "${PC_liblo_VERSION}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  liblo
  REQUIRED_VARS liblo_LIBRARY liblo_INCLUDE_DIR
  VERSION_VAR liblo_VERSION
)

if(liblo_FOUND)
  set(liblo_LIBRARIES "${liblo_LIBRARY}")
  set(liblo_INCLUDE_DIRS "${liblo_INCLUDE_DIR}")
  if(NOT TARGET liblo::liblo)
    add_library(liblo::liblo UNKNOWN IMPORTED)
    set_target_properties(
      liblo::liblo
      PROPERTIES
        IMPORTED_LOCATION "${liblo_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_liblo_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${liblo_INCLUDE_DIR}"
    )
  endif()
endif()
