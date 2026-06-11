#[=======================================================================[.rst:
Findsord
--------

Finds the sord library and its development headers.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``sord::sord``
  The sord library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``sord_FOUND``
  True if the system has the sord library.
``sord_INCLUDE_DIRS``
  Include directories needed to use sord.
``sord_LIBRARIES``
  Libraries needed to link to sord.
``sord_DEFINITIONS``
  Compile definitions needed to use sord.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``sord_INCLUDE_DIR``
  The directory containing ``sord/sord.h``.
``sord_LIBRARY``
  The path to the sord library.

#]=======================================================================]

include(IsStaticLibrary)

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_sord QUIET sord-0)
endif()

find_path(
  sord_INCLUDE_DIR
  NAMES sord/sord.h
  HINTS ${PC_sord_INCLUDE_DIRS}
  DOC "sord include directory"
)
mark_as_advanced(sord_INCLUDE_DIR)

find_library(
  sord_LIBRARY
  NAMES sord-0
  HINTS ${PC_sord_LIBRARY_DIRS}
  DOC "sord library"
)
mark_as_advanced(sord_LIBRARY)

if(DEFINED PC_sord_VERSION AND NOT PC_sord_VERSION STREQUAL "")
  set(sord_VERSION "${PC_sord_VERSION}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  sord
  REQUIRED_VARS sord_LIBRARY sord_INCLUDE_DIR
  VERSION_VAR sord_VERSION
)

if(sord_FOUND)
  set(sord_LIBRARIES "${sord_LIBRARY}")
  set(sord_INCLUDE_DIRS "${sord_INCLUDE_DIR}")
  set(sord_DEFINITIONS ${PC_sord_CFLAGS_OTHER})

  if(NOT TARGET sord::sord)
    add_library(sord::sord UNKNOWN IMPORTED)
    set_target_properties(
      sord::sord
      PROPERTIES
        IMPORTED_LOCATION "${sord_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_sord_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${sord_INCLUDE_DIR}"
    )
    is_static_library(sord_IS_STATIC sord::sord)
    if(sord_IS_STATIC)
      find_package(serd REQUIRED)
      set_property(
        TARGET sord::sord
        APPEND
        PROPERTY INTERFACE_LINK_LIBRARIES serd::serd
      )
      if(sord_VERSION VERSION_GREATER_EQUAL "0.16.16")
        find_package(zix REQUIRED)
        set_property(
          TARGET sord::sord
          APPEND
          PROPERTY INTERFACE_LINK_LIBRARIES zix::zix
        )
      endif()
    endif()
  endif()
endif()
