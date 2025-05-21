#[=======================================================================[.rst:
FindLibudev
--------

Finds the libudev (userspace device manager) library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``Libudev::Libudev``
  The libudev library

#]=======================================================================]

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_Libudev QUIET libudev)
endif()

find_path(
  Libudev_INCLUDE_DIR
  NAMES libudev.h
  PATHS ${PC_Libudev_INCLUDE_DIRS}
  DOC "The libudev include directory"
)
mark_as_advanced(Libudev_INCLUDE_DIR)

find_library(
  Libudev_LIBRARY
  NAMES udev PATH ${PC_Libudev_LIBRARY_DIRS}
  DOC "The libudev library"
)
mark_as_advanced(Libudev_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Libudev
  REQUIRED_VARS Libudev_LIBRARY Libudev_INCLUDE_DIR
  VERSION_VAR Libudev_VERSION
)

if(Libudev_FOUND)
  set(Libudev_LIBRARIES "${Libudev_LIBRARY}")
  set(Libudev_INCLUDE_DIRS "${Libudev_INCLUDE_DIR}")
  set(Libudev_DEFINITIONS ${PC_Libudev_CFLAGS_OTHER})

  if(NOT TARGET Libudev::Libudev)
    add_library(Libudev::Libudev UNKNOWN IMPORTED)
    set_target_properties(
      Libudev::Libudev
      PROPERTIES
        IMPORTED_LOCATION "${Libudev_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_Libudev_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${Libudev_INCLUDE_DIR}"
    )
  endif()
endif()
