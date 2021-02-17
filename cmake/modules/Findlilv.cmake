# This file is part of Mixxx, Digital DJ'ing software.
# Copyright (C) 2001-2020 Mixxx Development Team
# Distributed under the GNU General Public Licence (GPL) version 2 or any later
# later version. See the LICENSE file for details.

#[=======================================================================[.rst:
Findlilv
--------

Finds the lilv library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``lilv::lilv``
  The lilv library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``lilv_FOUND``
  True if the system has the lilv library.
``lilv_INCLUDE_DIRS``
  Include directories needed to use lilv.
``lilv_LIBRARIES``
  Libraries needed to link to lilv.
``lilv_DEFINITIONS``
  Compile definitions needed to use lilv.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``lilv_INCLUDE_DIR``
  The directory containing ``lilv-0/lilb/lilv.h``.
``lilv_LIBRARY``
  The path to the lilv library.

#]=======================================================================]

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_lilv QUIET lilv-0)
endif()

find_path(lilv_INCLUDE_DIR
  NAMES lilv/lilv.h
  PATH_SUFFIXES lilv-0
  PATHS ${PC_lilv_INCLUDE_DIRS}
  DOC "lilv include directory"
)
mark_as_advanced(lilv_INCLUDE_DIR)

find_library(lilv_LIBRARY
  NAMES lilv-0 lilv
  PATHS ${PC_lilv_LIBRARY_DIRS}
  DOC "lilv library"
)
mark_as_advanced(lilv_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  lilv
  DEFAULT_MSG
  lilv_LIBRARY
  lilv_INCLUDE_DIR
)

if(lilv_FOUND)
  set(lilv_LIBRARIES "${lilv_LIBRARY}")
  set(lilv_INCLUDE_DIRS "${lilv_INCLUDE_DIR}")
  set(lilv_DEFINITIONS ${PC_lilv_CFLAGS_OTHER})

  if(NOT TARGET lilv::lilv)
    add_library(lilv::lilv UNKNOWN IMPORTED)
    set_target_properties(lilv::lilv
      PROPERTIES
        IMPORTED_LOCATION "${lilv_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_lilv_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${lilv_INCLUDE_DIR}"
    )
  endif()
endif()
