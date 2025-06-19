#[=======================================================================[.rst:
Findserd
--------

Finds the serd library and its development headers.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``serd::serd``
  The serd library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``serd_FOUND``
  True if the system has the serd library.
``serd_INCLUDE_DIRS``
  Include directories needed to use serd.
``serd_LIBRARIES``
  Libraries needed to link to serd.
``serd_DEFINITIONS``
  Compile definitions needed to use serd.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``serd_INCLUDE_DIR``
  The directory containing ``serd/serd.h``.
``serd_LIBRARY``
  The path to the serd library.

#]=======================================================================]

include(IsStaticLibrary)

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_serd QUIET serd-0)
endif()

find_path(
  serd_INCLUDE_DIR
  NAMES serd/serd.h
  HINTS ${PC_serd_INCLUDE_DIRS}
  DOC "serd include directory"
)
mark_as_advanced(serd_INCLUDE_DIR)

find_library(
  serd_LIBRARY
  NAMES serd-0
  HINTS ${PC_serd_LIBRARY_DIRS}
  DOC "serd library"
)
mark_as_advanced(serd_LIBRARY)

if(DEFINED PC_serd_VERSION AND NOT PC_serd_VERSION STREQUAL "")
  set(serd_VERSION "${PC_serd_VERSION}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  serd
  REQUIRED_VARS serd_LIBRARY serd_INCLUDE_DIR
  VERSION_VAR serd_VERSION
)

if(serd_FOUND)
  set(serd_LIBRARIES "${serd_LIBRARY}")
  set(serd_INCLUDE_DIRS "${serd_INCLUDE_DIR}")
  set(serd_DEFINITIONS ${PC_serd_CFLAGS_OTHER})

  if(NOT TARGET serd::serd)
    add_library(serd::serd UNKNOWN IMPORTED)
    set_target_properties(
      serd::serd
      PROPERTIES
        IMPORTED_LOCATION "${serd_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_serd_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${serd_INCLUDE_DIR}"
    )
  endif()
endif()
