#[=======================================================================[.rst:
FindOpenMPT
-----------

Finds the OpenMPT library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``OpenMPT::OpenMPT``
  The OpenMPT library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``OpenMPT_FOUND``
  True if the system has the OpenMPT library.
``OpenMPT_INCLUDE_DIRS``
  Include directories needed to use OpenMPT.
``OpenMPT_LIBRARIES``
  Libraries needed to link to OpenMPT.
``OpenMPT_DEFINITIONS``
  Compile definitions needed to use OpenMPT.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``OpenMPT_INCLUDE_DIR``
  The directory containing ``libopenmpt/libopenmpt.hpp``.
``OpenMPT_LIBRARY``
  The path to the OpenMPT library.

#]=======================================================================]

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_OpenMPT QUIET libopenmpt)
endif()

find_path(
  OpenMPT_INCLUDE_DIR
  NAMES libopenmpt/libopenmpt.hpp libopenmpt/libopenmpt.h
  HINTS ${PC_OpenMPT_INCLUDE_DIRS}
  DOC "OpenMPT include directory"
)
mark_as_advanced(OpenMPT_INCLUDE_DIR)

find_library(
  OpenMPT_LIBRARY
  NAMES openmpt
  HINTS ${PC_OpenMPT_LIBRARY_DIRS}
  DOC "OpenMPT library"
)
mark_as_advanced(OpenMPT_LIBRARY)

if(DEFINED PC_OpenMPT_VERSION AND NOT PC_OpenMPT_VERSION STREQUAL "")
  set(OpenMPT_VERSION "${PC_OpenMPT_VERSION}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  OpenMPT
  REQUIRED_VARS OpenMPT_LIBRARY OpenMPT_INCLUDE_DIR
  VERSION_VAR OpenMPT_VERSION
)

if(OpenMPT_FOUND)
  set(OpenMPT_LIBRARIES "${OpenMPT_LIBRARY}")
  set(OpenMPT_INCLUDE_DIRS "${OpenMPT_INCLUDE_DIR}")
  set(OpenMPT_DEFINITIONS ${PC_OpenMPT_CFLAGS_OTHER})

  if(NOT TARGET OpenMPT::OpenMPT)
    add_library(OpenMPT::OpenMPT UNKNOWN IMPORTED)
    set_target_properties(
      OpenMPT::OpenMPT
      PROPERTIES
        IMPORTED_LOCATION "${OpenMPT_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_OpenMPT_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${OpenMPT_INCLUDE_DIR}"
    )
  endif()
endif()
