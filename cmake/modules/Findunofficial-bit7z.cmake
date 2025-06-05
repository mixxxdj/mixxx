#[=======================================================================[.rst:
Findunofficial-bit7z
-------------------

Finds the unofficial bit7z library.

Imported Targets
^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``unofficial::bit7z::bit7z64``
  The bit7z library

Result Variables
^^^^^^^^^^^^^^^

This will define the following variables:

``unofficial-bit7z_FOUND``
  True if the system has the bit7z library.
``unofficial-bit7z_INCLUDE_DIRS``
  Include directories needed to use bit7z.
``unofficial-bit7z_LIBRARIES``
  Libraries needed to link to bit7z.

Cache Variables
^^^^^^^^^^^^^^

The following cache variables may also be set:

``unofficial-bit7z_INCLUDE_DIR``
  The directory containing ``bit7z.hpp``.
``unofficial-bit7z_LIBRARY``
  The path to the bit7z library.

#]=======================================================================]

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_unofficial-bit7z QUIET unofficial-bit7z)
endif()

find_path(
  unofficial-bit7z_INCLUDE_DIR
  NAMES bit7z.hpp
  PATH_SUFFIXES bit7z
  HINTS ${PC_unofficial-bit7z_INCLUDE_DIRS}
  DOC "unofficial-bit7z include directory"
)
mark_as_advanced(unofficial-bit7z_INCLUDE_DIR)

find_library(
  unofficial-bit7z_LIBRARY
  NAMES bit7z
  HINTS ${PC_unofficial-bit7z_LIBRARY_DIRS}
  DOC "unofficial-bit7zlibrary"
)
mark_as_advanced(unofficial-bit7z_LIBRARY)

if(
  DEFINED PC_unofficial-bit7z_VERSION
  AND NOT PC_unofficial-bit7z_VERSION STREQUAL ""
)
  set(bit7z_VERSION "${PC_unofficial-bit7z_VERSION}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  bit7z
  REQUIRED_VARS unofficial-bit7z_LIBRARY unofficial-bit7z_INCLUDE_DIR
  VERSION_VAR unofficial-bit7z_VERSION
)

if(unofficial-bit7z_FOUND)
  if(NOT TARGET unofficial-bit7z::unofficial-bit7z)
    add_library(unofficial-bit7z::unofficial-bit7z UNKNOWN IMPORTED)
    set_target_properties(
      unofficial-bit7z::unofficial-bit7z
      PROPERTIES
        IMPORTED_LOCATION "${unofficial-bit7z_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_unofficial-bit7z_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${unofficial-bit7z_INCLUDE_DIR}"
    )
  endif()
endif()
