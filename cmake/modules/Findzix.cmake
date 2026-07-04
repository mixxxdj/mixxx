#[=======================================================================[.rst:
Findzix
--------

Finds the zix library and its development headers.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``zix::zix``
  The zix library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``zix_FOUND``
  True if the system has the zix library.
``zix_INCLUDE_DIRS``
  Include directories needed to use zix.
``zix_LIBRARIES``
  Libraries needed to link to zix.
``zix_DEFINITIONS``
  Compile definitions needed to use zix.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``zix_INCLUDE_DIR``
  The directory containing ``zix/zix.h``.
``zix_LIBRARY``
  The path to the zix library.

#]=======================================================================]

include(IsStaticLibrary)

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_zix QUIET zix-0)
endif()

find_path(
  zix_INCLUDE_DIR
  NAMES zix/zix.h
  HINTS ${PC_zix_INCLUDE_DIRS}
  DOC "zix include directory"
)
mark_as_advanced(zix_INCLUDE_DIR)

find_library(
  zix_LIBRARY
  NAMES zix-0
  HINTS ${PC_zix_LIBRARY_DIRS}
  DOC "zix library"
)
mark_as_advanced(zix_LIBRARY)

if(DEFINED PC_zix_VERSION AND NOT PC_zix_VERSION STREQUAL "")
  set(zix_VERSION "${PC_zix_VERSION}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  zix
  REQUIRED_VARS zix_LIBRARY zix_INCLUDE_DIR
  VERSION_VAR zix_VERSION
)

if(zix_FOUND)
  set(zix_LIBRARIES "${zix_LIBRARY}")
  set(zix_INCLUDE_DIRS "${zix_INCLUDE_DIR}")
  set(zix_DEFINITIONS ${PC_zix_CFLAGS_OTHER})

  if(NOT TARGET zix::zix)
    add_library(zix::zix UNKNOWN IMPORTED)
    set_target_properties(
      zix::zix
      PROPERTIES
        IMPORTED_LOCATION "${zix_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_zix_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${zix_INCLUDE_DIR}"
    )
  endif()
endif()
