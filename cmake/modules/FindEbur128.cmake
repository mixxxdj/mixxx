# This file is part of Mixxx, Digital DJ'ing software.
# Copyright (C) 2001-2020 Mixxx Development Team
# Distributed under the GNU General Public Licence (GPL) version 2 or any later
# later version. See the LICENSE file for details.

#[=======================================================================[.rst:
FindEbur128
-----------

Finds the Ebur128 library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``Ebur128::Ebur128``
  The Ebur128 library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``Ebur128_FOUND``
  True if the system has the Ebur128 library.
``Ebur128_INCLUDE_DIRS``
  Include directories needed to use Ebur128.
``Ebur128_LIBRARIES``
  Libraries needed to link to Ebur128.
``Ebur128_DEFINITIONS``
  Compile definitions needed to use Ebur128.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``Ebur128_INCLUDE_DIR``
  The directory containing ``ebur128.h``.
``Ebur128_LIBRARY``
  The path to the Ebur128 library.

#]=======================================================================]

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_Ebur128 QUIET libebur128)
endif()

find_path(Ebur128_INCLUDE_DIR
  NAMES ebur128.h
  PATHS ${PC_Ebur128_INCLUDE_DIRS}
  PATH_SUFFIXES ebur128
  DOC "Ebur128 include directory")
mark_as_advanced(Ebur128_INCLUDE_DIR)

find_library(Ebur128_LIBRARY
  NAMES ebur128
  PATHS ${PC_Ebur128_LIBRARY_DIRS}
  DOC "Ebur128 library"
)
mark_as_advanced(Ebur128_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Ebur128
  DEFAULT_MSG
  Ebur128_LIBRARY
  Ebur128_INCLUDE_DIR
)

if(Ebur128_FOUND)
  set(Ebur128_LIBRARIES "${Ebur128_LIBRARY}")
  set(Ebur128_INCLUDE_DIRS "${Ebur128_INCLUDE_DIR}")
  set(Ebur128_DEFINITIONS ${PC_Ebur128_CFLAGS_OTHER})

  if(NOT TARGET Ebur128::Ebur128)
    add_library(Ebur128::Ebur128 UNKNOWN IMPORTED)
    set_target_properties(Ebur128::Ebur128
      PROPERTIES
        IMPORTED_LOCATION "${Ebur128_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_Ebur128_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${Ebur128_INCLUDE_DIR}"
    )
  endif()
endif()
