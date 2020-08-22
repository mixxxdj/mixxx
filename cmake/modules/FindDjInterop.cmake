# This file is part of Mixxx, Digital DJ'ing software.
# Copyright (C) 2001-2020 Mixxx Development Team
# Distributed under the GNU General Public Licence (GPL) version 2 or any later
# later version. See the LICENSE file for details.

#[=======================================================================[.rst:
FindDjInterop
---------------

Finds the DjInterop library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``DjInterop::DjInterop``
  The DjInterop library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``DjInterop_FOUND``
  True if the system has the DjInterop library.
``DjInterop_INCLUDE_DIRS``
  Include directories needed to use DjInterop.
``DjInterop_LIBRARIES``
  Libraries needed to link to DjInterop.
``DjInterop_DEFINITIONS``
  Compile definitions needed to use DjInterop.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``DjInterop_INCLUDE_DIR``
  The directory containing ``djinterop/djinterop.hpp``.
``DjInterop_LIBRARY``
  The path to the DjInterop library.

#]=======================================================================]

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_DjInterop QUIET libdjinterop)
endif()

find_path(DjInterop_INCLUDE_DIR
  NAMES djinterop/djinterop.hpp
  PATHS ${PC_DjInterop_INCLUDE_DIRS}
  DOC "DjInterop include directory")
mark_as_advanced(DjInterop_INCLUDE_DIR)

find_library(DjInterop_LIBRARY
  NAMES djinterop
  PATHS ${PC_DjInterop_LIBRARY_DIRS}
  DOC "DjInterop library"
)
mark_as_advanced(DjInterop_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  DjInterop
  DEFAULT_MSG
  DjInterop_LIBRARY
  DjInterop_INCLUDE_DIR
)

if(DjInterop_FOUND)
  set(DjInterop_LIBRARIES "${DjInterop_LIBRARY}")
  set(DjInterop_INCLUDE_DIRS "${DjInterop_INCLUDE_DIR}")
  set(DjInterop_DEFINITIONS ${PC_DjInterop_CFLAGS_OTHER})

  if(NOT TARGET DjInterop::DjInterop)
    add_library(DjInterop::DjInterop UNKNOWN IMPORTED)
    set_target_properties(DjInterop::DjInterop
      PROPERTIES
        IMPORTED_LOCATION "${DjInterop_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_DjInterop_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${DjInterop_INCLUDE_DIR}"
    )
  endif()
endif()
