# This file is part of Mixxx, Digital DJ'ing software.
# Copyright (C) 2001-2020 Mixxx Development Team
# Distributed under the GNU General Public Licence (GPL) version 2 or any later
# later version. See the LICENSE file for details.

#[=======================================================================[.rst:
FindWavPack
-----------

Finds the WavPack library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``WavPack::WavPack``
  The WavPack library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``WavPack_FOUND``
  True if the system has the WavPack library.
``WavPack_INCLUDE_DIRS``
  Include directories needed to use WavPack.
``WavPack_LIBRARIES``
  Libraries needed to link to WavPack.
``WavPack_DEFINITIONS``
  Compile definitions needed to use WavPack.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``WavPack_INCLUDE_DIR``
  The directory containing ``wavpack/wavpack.h``.
``WavPack_LIBRARY``
  The path to the WavPack library.

#]=======================================================================]

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_WavPack QUIET wavpack)
endif()

find_path(WavPack_INCLUDE_DIR
  NAMES wavpack/wavpack.h
  PATHS ${PC_WavPack_INCLUDE_DIRS}
  DOC "WavPack include directory")
mark_as_advanced(WavPack_INCLUDE_DIR)

find_library(WavPack_LIBRARY
  NAMES wavpack wv
  PATHS ${PC_WavPack_LIBRARY_DIRS}
  DOC "WavPack library"
)
mark_as_advanced(WavPack_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  WavPack
  DEFAULT_MSG
  WavPack_LIBRARY
  WavPack_INCLUDE_DIR
)

if(WavPack_FOUND)
  set(WavPack_LIBRARIES "${WavPack_LIBRARY}")
  set(WavPack_INCLUDE_DIRS "${WavPack_INCLUDE_DIR}")
  set(WavPack_DEFINITIONS ${PC_WavPack_CFLAGS_OTHER})

  if(NOT TARGET WavPack::WavPack)
    add_library(WavPack::WavPack UNKNOWN IMPORTED)
    set_target_properties(WavPack::WavPack
      PROPERTIES
        IMPORTED_LOCATION "${WavPack_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_WavPack_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${WavPack_INCLUDE_DIR}"
    )
  endif()
endif()
