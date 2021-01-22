# This file is part of Mixxx, Digital DJ'ing software.
# Copyright (C) 2001-2020 Mixxx Development Team
# Distributed under the GNU General Public Licence (GPL) version 2 or any later
# later version. See the LICENSE file for details.

#[=======================================================================[.rst:
Findrubberband
--------------

Finds the rubberband library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``rubberband::rubberband``
  The rubberband library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``rubberband_FOUND``
  True if the system has the rubberband library.
``rubberband_INCLUDE_DIRS``
  Include directories needed to use rubberband.
``rubberband_LIBRARIES``
  Libraries needed to link to rubberband.
``rubberband_DEFINITIONS``
  Compile definitions needed to use rubberband.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``rubberband_INCLUDE_DIR``
  The directory containing ``rubberband/RubberBandStretcher.h``.
``rubberband_LIBRARY``
  The path to the rubberband library.

#]=======================================================================]

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_rubberband QUIET rubberband)
endif()

find_path(rubberband_INCLUDE_DIR
  NAMES rubberband/RubberBandStretcher.h
  PATHS ${PC_rubberband_INCLUDE_DIRS}
  DOC "rubberband include directory")
mark_as_advanced(rubberband_INCLUDE_DIR)

find_library(rubberband_LIBRARY
  NAMES rubberband rubberband-library rubberband-dll
  PATHS ${PC_rubberband_LIBRARY_DIRS}
  DOC "rubberband library"
)
mark_as_advanced(rubberband_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  rubberband
  DEFAULT_MSG
  rubberband_LIBRARY
  rubberband_INCLUDE_DIR
)

if(rubberband_FOUND)
  set(rubberband_LIBRARIES "${rubberband_LIBRARY}")
  set(rubberband_INCLUDE_DIRS "${rubberband_INCLUDE_DIR}")
  set(rubberband_DEFINITIONS ${PC_rubberband_CFLAGS_OTHER})

  if(NOT TARGET rubberband::rubberband)
    add_library(rubberband::rubberband UNKNOWN IMPORTED)
    set_target_properties(rubberband::rubberband
      PROPERTIES
        IMPORTED_LOCATION "${rubberband_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_rubberband_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${rubberband_INCLUDE_DIR}"
    )
  endif()
endif()
