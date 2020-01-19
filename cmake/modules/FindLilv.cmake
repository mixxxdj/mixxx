# This file is part of Mixxx, Digital DJ'ing software.
# Copyright (C) 2001-2020 Mixxx Development Team
# Distributed under the GNU General Public Licence (GPL) version 2 or any later
# later version. See the LICENSE file for details.

#[=======================================================================[.rst:
FindLilv
--------

Finds the Lilv library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``Lilv::Lilv``
  The Lilv library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``Lilv_FOUND``
  True if the system has the Lilv library.
``Lilv_INCLUDE_DIRS``
  Include directories needed to use Lilv.
``Lilv_LIBRARIES``
  Libraries needed to link to Lilv.
``Lilv_DEFINITIONS``
  Compile definitions needed to use Lilv.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``Lilv_INCLUDE_DIR``
  The directory containing ``lilv-0/lilb/lilv.h``.
``Lilv_LIBRARY``
  The path to the Lilv library.

#]=======================================================================]

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_Lilv QUIET lilv-0)
endif()

find_path(Lilv_INCLUDE_DIR
  NAMES lilv-0/lilv/lilv.h
  PATHS ${PC_Lilv_INCLUDE_DIRS}
  DOC "Lilv include directory"
)
mark_as_advanced(Lilv_INCLUDE_DIR)

find_library(Lilv_LIBRARY
  NAMES lilv-0
  PATHS ${PC_Lilv_LIBRARY_DIRS}
  DOC "Lilv library"
)
mark_as_advanced(Lilv_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Lilv
  DEFAULT_MSG
  Lilv_LIBRARY
  Lilv_INCLUDE_DIR
)

if(Lilv_FOUND)
  set(Lilv_LIBRARIES "${Lilv_LIBRARY}")
  set(Lilv_INCLUDE_DIRS "${Lilv_INCLUDE_DIR}")
  set(Lilv_DEFINITIONS ${PC_Lilv_CFLAGS_OTHER})

  if(NOT TARGET Lilv::Lilv)
    add_library(Lilv::Lilv UNKNOWN IMPORTED)
    set_target_properties(Lilv::Lilv
      PROPERTIES
        IMPORTED_LOCATION "${Lilv_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_Lilv_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${Lilv_INCLUDE_DIR}"
    )
  endif()
endif()
