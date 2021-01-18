# This file is part of Mixxx, Digital DJ'ing software.
# Copyright (C) 2001-2020 Mixxx Development Team
# Distributed under the GNU General Public Licence (GPL) version 2 or any later
# later version. See the LICENSE file for details.

#[=======================================================================[.rst:
FindGRANTLEE
--------

Finds the GRANTLEE library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``GRANTLEE::GRANTLEE``
  The GRANTLEE library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``GRANTLEE_FOUND``
  True if the system has the GRANTLEE library.
``GRANTLEE_INCLUDE_DIRS``
  Include directories needed to use GRANTLEE.
``GRANTLEE_LIBRARIES``
  Libraries needed to link to GRANTLEE.
``GRANTLEE_DEFINITIONS``
  Compile definitions needed to use GRANTLEE.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``GRANTLEE_INCLUDE_DIR``
  The directory containing ``GRANTLEE/all.h``.
``GRANTLEE_LIBRARY``
  The path to the GRANTLEE library.

#]=======================================================================]

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_GRANTLEE QUIET Grantlee5)
endif()

find_path(GRANTLEE_INCLUDE_DIR
  NAMES grantlee/parser.h
  PATHS ${PC_GRANTLEE_INCLUDE_DIRS}
  DOC "Grantlee include directory")
mark_as_advanced(GRANTLEE_INCLUDE_DIR)

find_library(GRANTLEE_LIBRARY
  NAMES GRANTLEE
  PATHS ${PC_GRANTLEE_LIBRARY_DIRS}
  DOC "Grantlee library"
)
mark_as_advanced(GRANTLEE_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  GRANTLEE
  DEFAULT_MSG
  GRANTLEE_LIBRARY
  GRANTLEE_INCLUDE_DIR
)

if(GRANTLEE_FOUND)
  set(GRANTLEE_LIBRARIES "${GRANTLEE_LIBRARY}")
  set(GRANTLEE_INCLUDE_DIRS "${GRANTLEE_INCLUDE_DIR}")
  set(GRANTLEE_DEFINITIONS ${PC_GRANTLEE_CFLAGS_OTHER})

  if(NOT TARGET GRANTLEE::GRANTLEE)
    add_library(GRANTLEE::GRANTLEE UNKNOWN IMPORTED)
    set_target_properties(GRANTLEE::GRANTLEE
      PROPERTIES
        IMPORTED_LOCATION "${GRANTLEE_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_GRANTLEE_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${GRANTLEE_INCLUDE_DIR}"
    )
  endif()
endif()
