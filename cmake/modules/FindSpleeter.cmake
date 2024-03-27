# This file is part of Mixxx, Digital DJ'ing software.
# Copyright (C) 2001-2020 Mixxx Development Team
# Distributed under the GNU General Public Licence (GPL) version 2 or any later
# later version. See the LICENSE file for details.

#[=======================================================================[.rst:
FindSpleeterpp
--------------

Finds the Spleeterpp library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``spleeter::Split`
  The Spleeterpp library

#]=======================================================================]

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_Spleeter QUIET spleeter)
endif()

find_path(Spleeter_INCLUDE_DIR
  NAMES spleeter.h
  PATHS ${PC_Spleeter_INCLUDE_DIRS}
  PATH_SUFFIXES spleeter
  DOC "Spleeter include directory")
mark_as_advanced(Spleeter_INCLUDE_DIR)

find_library(Spleeter_LIBRARY
  NAMES spleeter
  PATHS ${Spleeter_LIBRARY_DIRS}
  DOC "Spleeter library"
)
mark_as_advanced(Spleeterpp_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Spleeter
  DEFAULT_MSG
  Spleeter_LIBRARY
  Spleeter_INCLUDE_DIR
)

if(Spleeter_FOUND)
  set(Spleeter_LIBRARIES "${Spleeter_LIBRARY}")
  set(Spleeter_INCLUDE_DIRS "${Spleeter_INCLUDE_DIR}")
  set(Spleeter_DEFINITIONS ${PC_Spleeter_CFLAGS_OTHER})

  if(NOT TARGET spleeter::Split)
    add_library(spleeter::Split UNKNOWN IMPORTED)
    set_target_properties(spleeter::Split
      PROPERTIES
        IMPORTED_LOCATION "${Spleeter_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${Spleeter_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${Spleeter_INCLUDE_DIR}"
    )
  endif()
endif()
