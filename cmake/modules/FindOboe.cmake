#[=======================================================================[.rst:
FindOboe
--------

Finds the Oboe  library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``Oboe::Oboe``
  The Oboe library

#]=======================================================================]

# Prefer finding the libraries from pkgconfig rather than find_library. This is
# required to build with PipeWire's reimplementation of the Oboe library.
#
# This also enables using PortAudio with the Oboe port in vcpkg. That only
# builds OboeWeakAPI (not the Oboe server) which dynamically loads the real
# Oboe library and forwards API calls to it. OboeWeakAPI requires linking `dl`
# in addition to Oboe, as specified in the pkgconfig file in vcpkg.
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(Oboe Oboe)
endif()

find_path(
  Oboe_INCLUDE_DIR
  NAMES oboe/Oboe.h
  HINTS ${PC_Oboe_INCLUDE_DIRS}
  DOC "Oboe include directory"
)
mark_as_advanced(Oboe_INCLUDE_DIR)

find_library(
  Oboe_LIBRARY
  NAMES oboe
  HINTS ${PC_Oboe_LIBRARY_DIRS}
  DOC "Oboe library"
)
mark_as_advanced(Oboe_LIBRARY)

if(DEFINED PC_Oboe_VERSION AND NOT PC_Oboe_VERSION STREQUAL "")
  set(Oboe_VERSION "${PC_Oboe_VERSION}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Oboe
  REQUIRED_VARS Oboe_LIBRARY Oboe_INCLUDE_DIR
  VERSION_VAR Oboe_VERSION
)

if(Oboe_FOUND)
  set(Oboe_LIBRARIES "${Oboe_LIBRARY}")
  set(Oboe_INCLUDE_DIRS "${Oboe_INCLUDE_DIR}")
  set(Oboe_DEFINITIONS ${PC_Oboe_CFLAGS_OTHER})

  if(NOT TARGET Oboe::Oboe)
    add_library(Oboe::Oboe UNKNOWN IMPORTED)
    set_target_properties(
      Oboe::Oboe
      PROPERTIES
        IMPORTED_LOCATION "${Oboe_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_Oboe_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${Oboe_INCLUDE_DIR}"
    )
  endif()
endif()
