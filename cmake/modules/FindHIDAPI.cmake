# This file is part of Mixxx, Digital DJ'ing software.
# Copyright (C) 2001-2020 Mixxx Development Team
# Distributed under the GNU General Public Licence (GPL) version 2 or any later
# later version. See the LICENSE file for details.

#[=======================================================================[.rst:
FindHIDAPI
----------

Finds the HIDAPI library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``HIDAPI::libusb``
  The hidapi-libusb library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``HIDAPI_FOUND``
  True if the system has the HIDAPI library.
``HIDAPI_INCLUDE_DIRS``
  Include directories needed to use HIDAPI.
``HIDAPI_LIBRARIES``
  Libraries needed to link to HIDAPI.
``HIDAPI_DEFINITIONS``
  Compile definitions needed to use HIDAPI.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``HIDAPI_INCLUDE_DIR``
  The directory containing ``hidapi/hidapi.h``.
``HIDAPI_LIBRARY``
  The path to the hidapi-lbusb library.

#]=======================================================================]

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_HIDAPI QUIET hidapi-libusb)
endif()

find_path(HIDAPI_INCLUDE_DIR
  NAMES hidapi.h
  PATHS ${PC_HIDAPI_INCLUDE_DIRS}
  PATH_SUFFIXES hidapi
  DOC "HIDAPI include directory")
mark_as_advanced(HIDAPI_INCLUDE_DIR)

find_library(HIDAPI_LIBRARY
  NAMES hidapi-libusb
  PATHS ${PC_HIDAPI_LIBRARY_DIRS}
  DOC "HIDAPI library"
)
mark_as_advanced(HIDAPI_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  HIDAPI
  DEFAULT_MSG
  HIDAPI_LIBRARY
  HIDAPI_INCLUDE_DIR
)

if(HIDAPI_FOUND)
  set(HIDAPI_LIBRARIES "${HIDAPI_LIBRARY}")
  set(HIDAPI_INCLUDE_DIRS "${HIDAPI_INCLUDE_DIR}")
  set(HIDAPI_DEFINITIONS ${PC_HIDAPI_CFLAGS_OTHER})

  if(NOT TARGET HIDAPI::LibUSB)
    add_library(HIDAPI::LibUSB UNKNOWN IMPORTED)
    set_target_properties(HIDAPI::LibUSB
      PROPERTIES
        IMPORTED_LOCATION "${HIDAPI_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_HIDAPI_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${HIDAPI_INCLUDE_DIR}"
    )
  endif()
endif()
