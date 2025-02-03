# This file is part of Mixxx, Digital DJ'ing software.
# Copyright (C) 2001-2025 Mixxx Development Team
# Distributed under the GNU General Public Licence (GPL) version 2 or any later
# later version. See the LICENSE file for details.

#[=======================================================================[.rst:
FindKeyFinder
--------

Finds the KeyFinder library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``KeyFinder::KeyFinder``
  The KeyFinder library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``KeyFinder_FOUND``
  True if the system has the KeyFinder library.
``KeyFinder_INCLUDE_DIRS``
  Include directories needed to use KeyFinder.
``KeyFinder_LIBRARIES``
  Libraries needed to link to KeyFinder.
``KeyFinder_DEFINITIONS``
  Compile definitions needed to use KeyFinder.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``KeyFinder_INCLUDE_DIR``
  The directory containing ``keyfinder/keyfinder.h``.
``KeyFinder_LIBRARY``
  The path to the KeyFinder library.

#]=======================================================================]

include(IsStaticLibrary)

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_KeyFinder QUIET libKeyFinder>=2.0)
endif()

find_path(
  KeyFinder_INCLUDE_DIR
  NAMES keyfinder/keyfinder.h
  HINTS ${PC_KeyFinder_INCLUDE_DIRS}
  DOC "KeyFinder include directory"
)
mark_as_advanced(KeyFinder_INCLUDE_DIR)

find_library(
  KeyFinder_LIBRARY
  NAMES keyfinder
  HINTS ${PC_KeyFinder_LIBRARY_DIRS}
  DOC "KeyFinder library"
)
mark_as_advanced(KeyFinder_LIBRARY)

if(DEFINED PC_KeyFinder_VERSION AND NOT PC_KeyFinder_VERSION STREQUAL "")
  set(KeyFinder_VERSION "${PC_KeyFinder_VERSION}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  KeyFinder
  REQUIRED_VARS KeyFinder_LIBRARY KeyFinder_INCLUDE_DIR
  VERSION_VAR KeyFinder_VERSION
)

if(KeyFinder_FOUND)
  set(KeyFinder_LIBRARIES "${KeyFinder_LIBRARY}")
  set(KeyFinder_INCLUDE_DIRS "${KeyFinder_INCLUDE_DIR}")
  set(KeyFinder_DEFINITIONS ${PC_KeyFinder_CFLAGS_OTHER})

  if(NOT TARGET KeyFinder::KeyFinder)
    add_library(KeyFinder::KeyFinder UNKNOWN IMPORTED)
    set_target_properties(
      KeyFinder::KeyFinder
      PROPERTIES
        IMPORTED_LOCATION "${KeyFinder_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_KeyFinder_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${KeyFinder_INCLUDE_DIR}"
    )
    is_static_library(KeyFinder_IS_STATIC KeyFinder::KeyFinder)
    if(KeyFinder_IS_STATIC)
      find_package(FFTW3 REQUIRED)
      set_property(
        TARGET KeyFinder::KeyFinder
        APPEND
        PROPERTY INTERFACE_LINK_LIBRARIES FFTW3::fftw3
      )
    endif()
  endif()
endif()
