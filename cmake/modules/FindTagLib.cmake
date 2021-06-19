# This file is part of Mixxx, Digital DJ'ing software.
# Copyright (C) 2001-2020 Mixxx Development Team
# Distributed under the GNU General Public Licence (GPL) version 2 or any later
# later version. See the LICENSE file for details.

#[=======================================================================[.rst:
FindTagLib
-----------

Finds the TagLib library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``TagLib::TagLib``
  The TagLib library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``TagLib_FOUND``
  True if the system has the TagLib library.
``TagLib_INCLUDE_DIRS``
  Include directories needed to use TagLib.
``TagLib_LIBRARIES``
  Libraries needed to link to TagLib.
``TagLib_DEFINITIONS``
  Compile definitions needed to use TagLib.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``TagLib_INCLUDE_DIR``
  The directory containing ``ebur128.h``.
``TagLib_LIBRARY``
  The path to the TagLib library.

#]=======================================================================]

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_TagLib QUIET taglib)
endif()

find_path(TagLib_INCLUDE_DIR
  NAMES tag.h
  PATHS ${PC_TagLib_INCLUDE_DIRS}
  PATH_SUFFIXES taglib
  DOC "TagLib include directory")
mark_as_advanced(TagLib_INCLUDE_DIR)

find_library(TagLib_LIBRARY
  NAMES tag
  PATHS ${PC_TagLib_LIBRARY_DIRS}
  DOC "TagLib library"
)
mark_as_advanced(TagLib_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  TagLib
  DEFAULT_MSG
  TagLib_LIBRARY
  TagLib_INCLUDE_DIR
)

if(TagLib_FOUND)
  set(TagLib_LIBRARIES "${TagLib_LIBRARY}")
  set(TagLib_INCLUDE_DIRS "${TagLib_INCLUDE_DIR}")
  set(TagLib_DEFINITIONS ${PC_TagLib_CFLAGS_OTHER})

  if(NOT TARGET TagLib::TagLib)
    add_library(TagLib::TagLib UNKNOWN IMPORTED)
    set_target_properties(TagLib::TagLib
      PROPERTIES
        IMPORTED_LOCATION "${TagLib_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_TagLib_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${TagLib_INCLUDE_DIR}"
    )
  endif()
endif()
