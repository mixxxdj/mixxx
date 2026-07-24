#[=======================================================================[.rst:
FindBungee
----------

Finds the Bungee audio stretcher library.

This module is invoked via ``find_package(Bungee MODULE)``.  Before reaching
this module the caller should already have tried CONFIG-mode packages:

.. code-block:: cmake

  find_package(Bungee CONFIG QUIET)         # upstream config package
  find_package(unofficial-bungee CONFIG QUIET) # vcpkg PR #50120

This module handles the remaining discovery paths:

1. ``pkg-config`` module ``libbungee`` (installed by Bungee or its vcpkg port).
2. Manual ``find_path`` / ``find_library`` search (system installs,
  developer build directories).

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``Bungee::Bungee``
  The Bungee library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``Bungee_FOUND``
  True if the system has the Bungee library.
``Bungee_INCLUDE_DIRS``
  Include directories needed to use Bungee.
``Bungee_LIBRARIES``
  Libraries needed to link to Bungee.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``Bungee_INCLUDE_DIR``
  The directory containing ``bungee/Bungee.h``.
``Bungee_LIBRARY``
  The path to the Bungee library.

#]=======================================================================]

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_Bungee QUIET libbungee)
endif()

find_path(
  Bungee_INCLUDE_DIR
  NAMES bungee/Bungee.h
  HINTS
    ${PC_Bungee_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../bungee
    ${CMAKE_SOURCE_DIR}/../bungee
  DOC "Bungee include directory"
)
mark_as_advanced(Bungee_INCLUDE_DIR)

find_library(
  Bungee_LIBRARY
  NAMES bungee libbungee
  HINTS
    ${PC_Bungee_LIBRARY_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../bungee/build
    ${CMAKE_SOURCE_DIR}/../bungee/build
  DOC "Bungee library"
)
mark_as_advanced(Bungee_LIBRARY)

if(DEFINED PC_Bungee_VERSION AND NOT PC_Bungee_VERSION STREQUAL "")
  set(Bungee_VERSION "${PC_Bungee_VERSION}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Bungee
  REQUIRED_VARS Bungee_LIBRARY Bungee_INCLUDE_DIR
  VERSION_VAR Bungee_VERSION
)

if(Bungee_FOUND)
  set(Bungee_LIBRARIES "${Bungee_LIBRARY}")
  set(Bungee_INCLUDE_DIRS "${Bungee_INCLUDE_DIR}")

  if(NOT TARGET Bungee::Bungee)
    add_library(Bungee::Bungee UNKNOWN IMPORTED)
    set_target_properties(
      Bungee::Bungee
      PROPERTIES
        IMPORTED_LOCATION "${Bungee_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${Bungee_INCLUDE_DIR}"
    )
  endif()
endif()
