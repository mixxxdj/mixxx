# This file is part of Mixxx, Digital DJ'ing software.
# Copyright (C) 2001-2020 Mixxx Development Team
# Distributed under the GNU General Public Licence (GPL) version 2 or any later
# later version. See the LICENSE file for details.

#[=======================================================================[.rst:
Findhidapi
----------

Finds the hidapi library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``hidapi::libusb``
  The hidapi-libusb library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``hidapi_FOUND``
  True if the system has the hidapi library.
``hidapi_INCLUDE_DIRS``
  Include directories needed to use hidapi.
``hidapi_LIBRARIES``
  Libraries needed to link to hidapi.
``hidapi_DEFINITIONS``
  Compile definitions needed to use hidapi.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``hidapi_INCLUDE_DIR``
  The directory containing ``hidapi/hidapi.h``.
``hidapi_LIBRARY``
  The path to the hidapi-lbusb library.

#]=======================================================================]

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_hidapi QUIET hidapi-libusb)
endif()

find_path(hidapi_INCLUDE_DIR
  NAMES hidapi.h
  PATHS ${PC_hidapi_INCLUDE_DIRS}
  PATH_SUFFIXES hidapi
  DOC "hidapi include directory")
mark_as_advanced(hidapi_INCLUDE_DIR)

find_library(hidapi_LIBRARY
  NAMES hidapi-libusb hidapi
  PATHS ${PC_hidapi_LIBRARY_DIRS}
  DOC "hidapi library"
)
mark_as_advanced(hidapi_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  hidapi
  DEFAULT_MSG
  hidapi_LIBRARY
  hidapi_INCLUDE_DIR
)

if(hidapi_FOUND)
  set(hidapi_LIBRARIES "${hidapi_LIBRARY}")
  set(hidapi_INCLUDE_DIRS "${hidapi_INCLUDE_DIR}")
  set(hidapi_DEFINITIONS ${PC_hidapi_CFLAGS_OTHER})

  if(NOT TARGET hidapi::hidapi)
    add_library(hidapi::hidapi UNKNOWN IMPORTED)
    set_target_properties(hidapi::hidapi
      PROPERTIES
        IMPORTED_LOCATION "${hidapi_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_hidapi_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${hidapi_INCLUDE_DIR}"
    )
  endif()
endif()
