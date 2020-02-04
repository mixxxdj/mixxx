# This file is part of Mixxx, Digital DJ'ing software.
# Copyright (C) 2001-2020 Mixxx Development Team
# Distributed under the GNU General Public Licence (GPL) version 2 or any later
# later version. See the LICENSE file for details.

#[=======================================================================[.rst:
FindLAME
--------

Finds the LAME library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``LAME::LAME``
  The LAME library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``LAME_FOUND``
  True if the system has the LAME library.
``LAME_INCLUDE_DIRS``
  Include directories needed to use LAME.
``LAME_LIBRARIES``
  Libraries needed to link to LAME.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``LAME_INCLUDE_DIR``
  The directory containing ``lame/lame.h``.
``LAME_LIBRARY``
  The path to the LAME library.

#]=======================================================================]

find_path(LAME_INCLUDE_DIR
  NAMES lame/lame.h
  DOC "LAME include directory")
mark_as_advanced(LAME_INCLUDE_DIR)

find_library(LAME_LIBRARY
  NAMES mp3lame mp3lame-static
  DOC "LAME library"
)
mark_as_advanced(LAME_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  LAME
  DEFAULT_MSG
  LAME_LIBRARY
  LAME_INCLUDE_DIR
)

if(LAME_FOUND)
  set(LAME_LIBRARIES "${LAME_LIBRARY}")
  set(LAME_INCLUDE_DIRS "${LAME_INCLUDE_DIR}")

  if(NOT TARGET LAME::LAME)
    add_library(LAME::LAME UNKNOWN IMPORTED)
    set_target_properties(LAME::LAME
      PROPERTIES
        IMPORTED_LOCATION "${LAME_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${LAME_INCLUDE_DIR}"
    )
  endif()
endif()
