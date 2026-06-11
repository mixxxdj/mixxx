#[=======================================================================[.rst:
FindLibUSB
----------

Finds the LibUSB library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``LibUSB::LibUSB``
  The LibUSB library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``LibUSB_FOUND``
  True if the system has the LibUSB library.
``LibUSB_INCLUDE_DIRS``
  Include directories needed to use LibUSB.
``LibUSB_LIBRARIES``
  Libraries needed to link to LibUSB.
``LibUSB_DEFINITIONS``
  Compile definitions needed to use LibUSB.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``LibUSB_INCLUDE_DIR``
  The directory containing ``libusb-1.0/libusb.h``.
``LibUSB_LIBRARY``
  The path to the LibUSB library.

#]=======================================================================]

include(IsStaticLibrary)

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_LibUSB QUIET libusb-1.0)
endif()

find_path(
  LibUSB_INCLUDE_DIR
  NAMES libusb.h
  PATH_SUFFIXES libusb libusb-1.0
  HINTS ${PC_LibUSB_INCLUDE_DIRS}
  DOC "LibUSB include directory"
)
mark_as_advanced(LibUSB_INCLUDE_DIR)

find_library(
  LibUSB_LIBRARY
  NAMES usb-1.0 usb
  HINTS ${PC_LibUSB_LIBRARY_DIRS}
  DOC "LibUSB library"
)
mark_as_advanced(LibUSB_LIBRARY)

if(DEFINED PC_LibUSB_VERSION AND NOT PC_LibUSB_VERSION STREQUAL "")
  set(LibUSB_VERSION "${PC_LibUSB_VERSION}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  LibUSB
  REQUIRED_VARS LibUSB_LIBRARY LibUSB_INCLUDE_DIR
  VERSION_VAR LibUSB_VERSION
)

if(LibUSB_FOUND)
  set(LibUSB_LIBRARIES "${LibUSB_LIBRARY}")
  set(LibUSB_INCLUDE_DIRS "${LibUSB_INCLUDE_DIR}")
  set(LibUSB_DEFINITIONS ${PC_LibUSB_CFLAGS_OTHER})

  if(NOT TARGET LibUSB::LibUSB)
    add_library(LibUSB::LibUSB UNKNOWN IMPORTED)
    set_target_properties(
      LibUSB::LibUSB
      PROPERTIES
        IMPORTED_LOCATION "${LibUSB_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_LibUSB_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${LibUSB_INCLUDE_DIR}"
    )

    is_static_library(LibUSB_IS_STATIC LibUSB::LibUSB)
    if(LibUSB_IS_STATIC)
      find_package(Libudev)
      if(Libudev_FOUND)
        set_property(
          TARGET LibUSB::LibUSB
          APPEND
          PROPERTY INTERFACE_LINK_LIBRARIES Libudev::Libudev
        )
      endif()
    endif()
  endif()
endif()
