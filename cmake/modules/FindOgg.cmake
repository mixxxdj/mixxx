# This file is part of Mixxx, Digital DJ'ing software.
# Copyright (C) 2001-2020 Mixxx Development Team
# Distributed under the GNU General Public Licence (GPL) version 2 or any later
# later version. See the LICENSE file for details.

#[=======================================================================[.rst:
FindOgg
-------

Finds the Ogg library.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``Ogg_FOUND``
  True if the system has the Ogg library.
``Ogg_INCLUDE_DIRS``
  Include directories needed to use Ogg.
``Ogg_LIBRARIES``
  Libraries needed to link to Ogg.
``Ogg_DEFINITIONS``
  Compile definitions needed to use Ogg.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``Ogg_INCLUDE_DIR``
  The directory containing ``ogg/ogg.h``.
``Ogg_LIBRARY``
  The path to the Ogg library.

#]=======================================================================]
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_Ogg QUIET ogg)
endif()

find_path(Ogg_INCLUDE_DIR
  NAMES ogg/ogg.h
  PATHS ${PC_Ogg_INCLUDE_DIRS}
  DOC "Ogg include directory"
)
mark_as_advanced(Ogg_INCLUDE_DIR)

find_library(Ogg_LIBRARY
  NAMES ogg
  PATHS ${PC_Ogg_LIBRARY_DIRS}
  DOC "Ogg library"
)
mark_as_advanced(Ogg_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Ogg
  REQUIRED_VARS
  Ogg_INCLUDE_DIR
  Ogg_LIBRARY
)

if(Ogg_FOUND)
  set(Ogg_LIBRARIES ${Ogg_LIBRARY})
  set(Ogg_INCLUDE_DIRS ${Ogg_INCLUDE_DIR})
  set(Ogg_DEFINITIONS ${PC_Ogg_CFLAGS_OTHER})

  if(NOT TARGET Ogg::ogg)
    add_library(Ogg::ogg UNKNOWN IMPORTED)
    set_target_properties(Ogg::ogg
      PROPERTIES
        IMPORTED_LOCATION "${Ogg_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_Ogg_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${Ogg_INCLUDE_DIR}"
    )
  endif()
endif()
