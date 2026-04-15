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

include(IsStaticLibrary)

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_search_module(PC_hidapi QUIET hidapi-libusb hidapi)
endif()

find_path(
  hidapi_INCLUDE_DIR
  NAMES hidapi.h
  HINTS ${PC_hidapi_INCLUDE_DIRS}
  PATH_SUFFIXES hidapi
  DOC "hidapi include directory"
)
mark_as_advanced(hidapi_INCLUDE_DIR)

find_library(
  hidapi_LIBRARY
  NAMES hidapi-libusb hidapi
  HINTS ${PC_hidapi_LIBRARY_DIRS}
  DOC "hidapi library"
)
mark_as_advanced(hidapi_LIBRARY)

if(CMAKE_SYSTEM_NAME STREQUAL Linux)
  find_library(
    hidapi-hidraw_LIBRARY
    NAMES hidapi-hidraw
    HINTS ${PC_hidapi_LIBRARY_DIRS}
    DOC "hidap-hidraw library"
  )
  mark_as_advanced(hidapi-hidraw_LIBRARY)
endif()

# Version detection
if(DEFINED PC_hidapi_VERSION AND NOT PC_hidapi_VERSION STREQUAL "")
  set(hidapi_VERSION "${PC_hidapi_VERSION}")
else()
  if(EXISTS "${hidapi_LIBRARY}" AND EXISTS "${hidapi_INCLUDE_DIR}/hidapi.h")
    file(READ "${hidapi_INCLUDE_DIR}/hidapi.h" hidapi_H_CONTENTS)
    string(
      REGEX MATCH
      "#define HID_API_VERSION_MAJOR ([0-9]+)"
      _dummy
      "${hidapi_H_CONTENTS}"
    )
    set(hidapi_VERSION_MAJOR "${CMAKE_MATCH_1}")
    string(
      REGEX MATCH
      "#define HID_API_VERSION_MINOR ([0-9]+)"
      _dummy
      "${hidapi_H_CONTENTS}"
    )
    set(hidapi_VERSION_MINOR "${CMAKE_MATCH_1}")
    string(
      REGEX MATCH
      "#define HID_API_VERSION_PATCH ([0-9]+)"
      _dummy
      "${hidapi_H_CONTENTS}"
    )
    set(hidapi_VERSION_PATCH "${CMAKE_MATCH_1}")
    # hidapi_VERSION is only available starting with 0.10.0
    # Simply using if(NOT) does not work because 0 is a valid value, so compare to empty string.
    if(
      NOT hidapi_VERSION_MAJOR STREQUAL ""
      AND NOT hidapi_VERSION_MINOR STREQUAL ""
      AND NOT hidapi_VERSION_PATCH STREQUAL ""
    )
      set(
        hidapi_VERSION
        "${hidapi_VERSION_MAJOR}.${hidapi_VERSION_MINOR}.${hidapi_VERSION_PATCH}"
      )
    endif()
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  hidapi
  REQUIRED_VARS hidapi_LIBRARY hidapi_INCLUDE_DIR hidapi_VERSION
  VERSION_VAR hidapi_VERSION
)

if(hidapi_FOUND)
  set(hidapi_LIBRARIES "${hidapi_LIBRARY}")
  set(hidapi_INCLUDE_DIRS "${hidapi_INCLUDE_DIR}")
  set(hidapi_DEFINITIONS ${PC_hidapi_CFLAGS_OTHER})

  if(NOT TARGET hidapi::hidapi)
    add_library(hidapi::hidapi UNKNOWN IMPORTED)
    set_target_properties(
      hidapi::hidapi
      PROPERTIES
        IMPORTED_LOCATION "${hidapi_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_hidapi_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${hidapi_INCLUDE_DIR}"
    )
    if(CMAKE_SYSTEM_NAME STREQUAL Linux)
      add_library(hidapi::hidraw UNKNOWN IMPORTED)
      set_target_properties(
        hidapi::hidraw
        PROPERTIES
          IMPORTED_LOCATION "${hidapi-hidraw_LIBRARY}"
          INTERFACE_COMPILE_OPTIONS "${PC_hidapi_CFLAGS_OTHER}"
          INTERFACE_INCLUDE_DIRECTORIES "${hidapi_INCLUDE_DIR}"
      )

      find_package(Libudev)
      if(Libudev_FOUND)
        is_static_library(hidapi_IS_STATIC hidapi::hidapi)
        if(hidapi_IS_STATIC)
          set_property(
            TARGET hidapi::hidapi
            APPEND
            PROPERTY INTERFACE_LINK_LIBRARIES Libudev::Libudev
          )
        endif()

        is_static_library(hidapi-hidraw_IS_STATIC hidapi::hidraw)
        if(hidapi-hidraw_IS_STATIC)
          set_property(
            TARGET hidapi::hidraw
            APPEND
            PROPERTY INTERFACE_LINK_LIBRARIES Libudev::Libudev
          )
        endif()
      endif()
    endif()
  endif()
endif()
