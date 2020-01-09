# This file is part of Mixxx, Digital DJ'ing software.
# Copyright (C) 2001-2020 Mixxx Development Team
# Distributed under the GNU General Public Licence (GPL) version 2 or any later
# later version. See the LICENSE file for details.

#[=======================================================================[.rst:
FindUpower
----------

Finds the Upower library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``Upower::Upower``
  The Upower library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``Upower_FOUND``
  True if the system has the Upower library.
``Upower_INCLUDE_DIRS``
  Include directories needed to use Upower.
``Upower_LIBRARIES``
  Libraries needed to link to Upower.
``Upower_DEFINITIONS``
  Compile definitions needed to use Upower.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``Upower_INCLUDE_DIR``
  The directory containing ``libupower-glib/upower.h``.
``Upower_LIBRARY``
  The path to the Upower library.

#]=======================================================================]

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_Upower QUIET upower-glib)
endif()

find_path(Upower_INCLUDE_DIR
  NAMES upower.h
  PATH_SUFFIXES upower-glib libupower-glib
  PATHS ${PC_Upower_INCLUDE_DIRS}
  DOC "Upower include directory")
mark_as_advanced(Upower_INCLUDE_DIR)

find_library(Upower_LIBRARY
  NAMES upower-glib
  PATHS ${PC_Upower_LIBRARY_DIRS}
  DOC "Upower library"
)
mark_as_advanced(Upower_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Upower
  DEFAULT_MSG
  Upower_LIBRARY
  Upower_INCLUDE_DIR
)

if(Upower_FOUND)
  set(Upower_LIBRARIES "${Upower_LIBRARY}")
  set(Upower_INCLUDE_DIRS "${Upower_INCLUDE_DIR}")
  set(Upower_DEFINITIONS ${PC_Upower_CFLAGS_OTHER})

  if(NOT TARGET Upower::Upower)
    add_library(Upower::Upower UNKNOWN IMPORTED)
    set_target_properties(Upower::Upower
      PROPERTIES
        IMPORTED_LOCATION "${Upower_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_Upower_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${Upower_INCLUDE_DIR}"
    )
  endif()
endif()
