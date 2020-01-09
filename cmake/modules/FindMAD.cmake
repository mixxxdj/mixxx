# This file is part of Mixxx, Digital DJ'ing software.
# Copyright (C) 2001-2020 Mixxx Development Team
# Distributed under the GNU General Public Licence (GPL) version 2 or any later
# later version. See the LICENSE file for details.

#[=======================================================================[.rst:
FindMAD
-------

Finds the MAD library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``MAD::MAD``
  The MAD library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``MAD_FOUND``
  True if the system has the MAD library.
``MAD_INCLUDE_DIRS``
  Include directories needed to use MAD.
``MAD_LIBRARIES``
  Libraries needed to link to MAD.
``MAD_DEFINITIONS``
  Compile definitions needed to use MAD.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``MAD_INCLUDE_DIR``
  The directory containing ``mad.h``.
``MAD_LIBRARY``
  The path to the MAD library.

#]=======================================================================]

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_MAD QUIET mad)
endif()

find_path(MAD_INCLUDE_DIR
  NAMES mad.h
  PATHS ${PC_MAD_INCLUDE_DIRS}
  PATH_SUFFIXES mad
  DOC "MAD include directory")
mark_as_advanced(MAD_INCLUDE_DIR)

find_library(MAD_LIBRARY
  NAMES mad
  PATHS ${PC_MAD_LIBRARY_DIRS}
  DOC "MAD library"
)
mark_as_advanced(MAD_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  MAD
  DEFAULT_MSG
  MAD_LIBRARY
  MAD_INCLUDE_DIR
)

if(MAD_FOUND)
  set(MAD_LIBRARIES "${MAD_LIBRARY}")
  set(MAD_INCLUDE_DIRS "${MAD_INCLUDE_DIR}")
  set(MAD_DEFINITIONS ${PC_MAD_CFLAGS_OTHER})

  if(NOT TARGET MAD::MAD)
    add_library(MAD::MAD UNKNOWN IMPORTED)
    set_target_properties(MAD::MAD
      PROPERTIES
        IMPORTED_LOCATION "${MAD_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_MAD_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${MAD_INCLUDE_DIR}"
    )
  endif()
endif()
