# This file is part of Mixxx, Digital DJ'ing software.
# Copyright (C) 2001-2020 Mixxx Development Team
# Distributed under the GNU General Public Licence (GPL) version 2 or any later
# later version. See the LICENSE file for details.

#[=======================================================================[.rst:
FindShout
---------

Finds the Shout library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``Shout::Shout``
  The Shout library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``Shout_FOUND``
  True if the system has the Shout library.
``Shout_INCLUDE_DIRS``
  Include directories needed to use Shout.
``Shout_LIBRARIES``
  Libraries needed to link to Shout.
``Shout_DEFINITIONS``
  Compile definitions needed to use Shout.
``Shout_VERSION``
  Shout Version.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``Shout_INCLUDE_DIR``
  The directory containing ``shout/shout.h``.
``Shout_LIBRARY``
  The path to the Shout library.

#]=======================================================================]

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_Shout QUIET shout)
endif()

find_path(Shout_INCLUDE_DIR
  NAMES shout/shout.h
  PATHS ${PC_Shout_INCLUDE_DIRS}
  DOC "Shout include directory")
mark_as_advanced(Shout_INCLUDE_DIR)

find_library(Shout_LIBRARY
  NAMES shout
  PATHS ${PC_Shout_LIBRARY_DIRS}
  DOC "Shout library"
)
mark_as_advanced(Shout_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Shout
  DEFAULT_MSG
  Shout_LIBRARY
  Shout_INCLUDE_DIR
)

if(Shout_FOUND)
  set(Shout_LIBRARIES "${Shout_LIBRARY}")
  set(Shout_INCLUDE_DIRS "${Shout_INCLUDE_DIR}")
  set(Shout_DEFINITIONS ${PC_Shout_CFLAGS_OTHER})
  set(Shout_VERSION ${PC_Shout_VERSION})

  if(NOT TARGET Shout::Shout)
    add_library(Shout::Shout UNKNOWN IMPORTED)
    set_target_properties(Shout::Shout
      PROPERTIES
        IMPORTED_LOCATION "${Shout_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_Shout_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${Shout_INCLUDE_DIR}"
    )
  endif()
endif()
