# This file is part of Mixxx, Digital DJ'ing software.
# Copyright (C) 2001-2020 Mixxx Development Team
# Distributed under the GNU General Public Licence (GPL) version 2 or any later
# later version. See the LICENSE file for details.

#[=======================================================================[.rst:
FindModplug
-----------

Finds the Modplug library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``Modplug::Modplug``
  The Modplug library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``Modplug_FOUND``
  True if the system has the Modplug library.
``Modplug_INCLUDE_DIRS``
  Include directories needed to use Modplug.
``Modplug_LIBRARIES``
  Libraries needed to link to Modplug.
``Modplug_DEFINITIONS``
  Compile definitions needed to use Modplug.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``Modplug_INCLUDE_DIR``
  The directory containing ``modplug.h``.
``Modplug_LIBRARY``
  The path to the Modplug library.

#]=======================================================================]

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_Modplug QUIET libmodplug)
endif()

find_path(Modplug_INCLUDE_DIR
  NAMES libmodplug/modplug.h
  PATHS ${PC_Modplug_INCLUDE_DIRS}
  DOC "Modplug include directory")
mark_as_advanced(Modplug_INCLUDE_DIR)

find_library(Modplug_LIBRARY
  NAMES modplug
  PATHS ${PC_Modplug_LIBRARY_DIRS}
  DOC "Modplug library"
)
mark_as_advanced(Modplug_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Modplug
  DEFAULT_MSG
  Modplug_LIBRARY
  Modplug_INCLUDE_DIR
)

if(Modplug_FOUND)
  set(Modplug_LIBRARIES "${Modplug_LIBRARY}")
  set(Modplug_INCLUDE_DIRS "${Modplug_INCLUDE_DIR}")
  set(Modplug_DEFINITIONS ${PC_Modplug_CFLAGS_OTHER})

  if(NOT TARGET Modplug::Modplug)
    add_library(Modplug::Modplug UNKNOWN IMPORTED)
    set_target_properties(Modplug::Modplug
      PROPERTIES
        IMPORTED_LOCATION "${Modplug_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_Modplug_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${Modplug_INCLUDE_DIR}"
    )
  endif()
endif()
