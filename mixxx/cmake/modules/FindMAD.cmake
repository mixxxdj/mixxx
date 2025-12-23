#[=======================================================================[.rst:
FindMAD
-------

Finds the MAD library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``MAD::MAD``
  The MAD library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``MAD_FOUND``
  True if the system has the MAD library.
``MAD_INCLUDE_DIRS``
  Include directories needed to use MAD.
``MAD_LIBRARIES``
  Libraries needed to link to MAD.
``MAD_DEFINITIONS``
  Compile definitions needed to use MAD.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``MAD_INCLUDE_DIR``
  The directory containing ``mad.h``.
``MAD_LIBRARY``
  The path to the MAD library.

#]=======================================================================]

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_MAD QUIET mad)
endif()

find_path(
  MAD_INCLUDE_DIR
  NAMES mad.h
  HINTS ${PC_MAD_INCLUDE_DIRS}
  PATH_SUFFIXES mad
  DOC "MAD include directory"
)
mark_as_advanced(MAD_INCLUDE_DIR)

find_library(
  MAD_LIBRARY
  NAMES mad
  HINTS ${PC_MAD_LIBRARY_DIRS}
  DOC "MAD library"
)
mark_as_advanced(MAD_LIBRARY)

if(DEFINED PC_MAD_VERSION AND NOT PC_MAD_VERSION STREQUAL "")
  set(MAD_VERSION "${PC_MAD_VERSION}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  MAD
  REQUIRED_VARS MAD_LIBRARY MAD_INCLUDE_DIR
  VERSION_VAR MAD_VERSION
)

if(MAD_FOUND)
  set(MAD_LIBRARIES "${MAD_LIBRARY}")
  set(MAD_INCLUDE_DIRS "${MAD_INCLUDE_DIR}")
  set(MAD_DEFINITIONS ${PC_MAD_CFLAGS_OTHER})

  if(NOT TARGET MAD::MAD)
    add_library(MAD::MAD UNKNOWN IMPORTED)
    set_target_properties(
      MAD::MAD
      PROPERTIES
        IMPORTED_LOCATION "${MAD_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_MAD_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${MAD_INCLUDE_DIR}"
    )
  endif()
endif()
