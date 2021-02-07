# This file is part of Mixxx, Digital DJ'ing software.
# Copyright (C) 2001-2020 Mixxx Development Team
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

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_KeyFinder QUIET libKeyFinder>=2.0)
endif()

find_path(KeyFinder_INCLUDE_DIR
  NAMES keyfinder/keyfinder.h
  PATHS ${PC_KeyFinder_INCLUDE_DIRS}
  DOC "KeyFinder include directory")
mark_as_advanced(KeyFinder_INCLUDE_DIR)

find_library(KeyFinder_LIBRARY
  NAMES keyfinder
  PATHS ${PC_KeyFinder_LIBRARY_DIRS}
  DOC "KeyFinder library"
)
mark_as_advanced(KeyFinder_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  KeyFinder
  DEFAULT_MSG
  KeyFinder_LIBRARY
  KeyFinder_INCLUDE_DIR
)

if(KeyFinder_FOUND)
  set(KeyFinder_LIBRARIES "${KeyFinder_LIBRARY}")
  set(KeyFinder_INCLUDE_DIRS "${KeyFinder_INCLUDE_DIR}")
  set(KeyFinder_DEFINITIONS ${PC_KeyFinder_CFLAGS_OTHER})

  if(NOT TARGET KeyFinder::KeyFinder)
    add_library(KeyFinder::KeyFinder UNKNOWN IMPORTED)
    set_target_properties(KeyFinder::KeyFinder
      PROPERTIES
        IMPORTED_LOCATION "${KeyFinder_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_KeyFinder_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${KeyFinder_INCLUDE_DIR}"
    )
  endif()
endif()
