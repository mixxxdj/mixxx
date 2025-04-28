#[=======================================================================[.rst:
FindMP4v2
---------

Finds the MP4v2 library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``MP4v2::MP4v2``
  The MP4v2 library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``MP4v2_FOUND``
  True if the system has the MP4v2 library.
``MP4v2_INCLUDE_DIRS``
  Include directories needed to use MP4v2.
``MP4v2_LIBRARIES``
  Libraries needed to link to MP4v2.
``MP4v2_DEFINITIONS``
  Compile definitions needed to use MP4v2.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``MP4v2_INCLUDE_DIR``
  The directory containing ``mp4v2/mp4v2.h``.
``MP4v2_LIBRARY``
  The path to the MP4v2 library.

#]=======================================================================]

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_MP4v2 QUIET mp4v2)
endif()

find_path(
  MP4v2_INCLUDE_DIR
  NAMES mp4v2/mp4v2.h
  HINTS ${PC_MP4v2_INCLUDE_DIRS}
  DOC "MP4v2 include directory"
)
mark_as_advanced(MP4v2_INCLUDE_DIR)

find_library(
  MP4v2_LIBRARY
  NAMES mp4v2
  HINTS ${PC_MP4v2_LIBRARY_DIRS}
  DOC "MP4v2 library"
)
mark_as_advanced(MP4v2_LIBRARY)

if(DEFINED PC_MP4v2_VERSION AND NOT PC_MP4v2_VERSION STREQUAL "")
  set(MP4v2_VERSION "${PC_MP4v2_VERSION}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  MP4v2
  REQUIRED_VARS MP4v2_LIBRARY MP4v2_INCLUDE_DIR
  VERSION_VAR MP4v2_VERSION
)

if(MP4v2_FOUND)
  set(MP4v2_LIBRARIES "${MP4v2_LIBRARY}")
  set(MP4v2_INCLUDE_DIRS "${MP4v2_INCLUDE_DIR}")
  set(MP4v2_DEFINITIONS ${PC_MP4v2_CFLAGS_OTHER})

  if(NOT TARGET MP4v2::MP4v2)
    add_library(MP4v2::MP4v2 UNKNOWN IMPORTED)
    set_target_properties(
      MP4v2::MP4v2
      PROPERTIES
        IMPORTED_LOCATION "${MP4v2_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_MP4v2_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${MP4v2_INCLUDE_DIR}"
    )
  endif()
endif()
