# This file is part of Mixxx, Digital DJ'ing software.
# Copyright (C) 2001-2020 Mixxx Development Team
# Distributed under the GNU General Public Licence (GPL) version 2 or any later
# later version. See the LICENSE file for details.

#[=======================================================================[.rst:
FindRubberband
--------------

Finds the Rubberband library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``Rubberband::Rubberband``
  The Rubberband library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``Rubberband_FOUND``
  True if the system has the Rubberband library.
``Rubberband_INCLUDE_DIRS``
  Include directories needed to use Rubberband.
``Rubberband_LIBRARIES``
  Libraries needed to link to Rubberband.
``Rubberband_DEFINITIONS``
  Compile definitions needed to use Rubberband.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``Rubberband_INCLUDE_DIR``
  The directory containing ``rubberband/RubberBandStretcher.h``.
``Rubberband_LIBRARY``
  The path to the Rubberband library.

#]=======================================================================]

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_Rubberband QUIET rubberband)
endif()

find_path(Rubberband_INCLUDE_DIR
  NAMES rubberband/RubberBandStretcher.h
  PATHS ${PC_Rubberband_INCLUDE_DIRS}
  DOC "Rubberband include directory")
mark_as_advanced(Rubberband_INCLUDE_DIR)

find_library(Rubberband_LIBRARY
  NAMES rubberband
  PATHS ${PC_Rubberband_LIBRARY_DIRS}
  DOC "Rubberband library"
)
mark_as_advanced(Rubberband_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Rubberband
  DEFAULT_MSG
  Rubberband_LIBRARY
  Rubberband_INCLUDE_DIR
)

if(Rubberband_FOUND)
  set(Rubberband_LIBRARIES "${Rubberband_LIBRARY}")
  set(Rubberband_INCLUDE_DIRS "${Rubberband_INCLUDE_DIR}")
  set(Rubberband_DEFINITIONS ${PC_Rubberband_CFLAGS_OTHER})

  if(NOT TARGET Rubberband::Rubberband)
    add_library(Rubberband::Rubberband UNKNOWN IMPORTED)
    set_target_properties(Rubberband::Rubberband
      PROPERTIES
        IMPORTED_LOCATION "${Rubberband_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_Rubberband_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${Rubberband_INCLUDE_DIR}"
    )
  endif()
endif()
