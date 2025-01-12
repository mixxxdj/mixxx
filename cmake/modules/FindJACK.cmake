#[=======================================================================[.rst:
FindJACK
--------

Finds the JACK Audio Connection Kit library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``JACK::jack``
  The JACK library

#]=======================================================================]

# Prefer finding the libraries from pkgconfig rather than find_library. This is
# required to build with PipeWire's reimplementation of the JACK library.
#
# This also enables using PortAudio with the jack2 port in vcpkg. That only
# builds JackWeakAPI (not the JACK server) which dynamically loads the real
# JACK library and forwards API calls to it. JackWeakAPI requires linking `dl`
# in addition to jack, as specified in the pkgconfig file in vcpkg.
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(JACK jack)
endif()

find_path(
  JACK_INCLUDE_DIR
  NAMES jack/jack.h
  HINTS ${PC_JACK_INCLUDE_DIRS}
  DOC "JACK include directory"
)
mark_as_advanced(JACK_INCLUDE_DIR)

find_library(
  JACK_LIBRARY
  NAMES jack
  HINTS ${PC_JACK_LIBRARY_DIRS}
  DOC "JACK library"
)
mark_as_advanced(JACK_LIBRARY)

if(WIN32)
  # vcpkg provides CMake targets for pthreads4w
  # This won't work if pthreads4w was built without vcpkg.
  find_package(pthreads REQUIRED)
  list(APPEND JACK_LINK_LIBRARIES PThreads4W::PThreads4W)
endif()

if(DEFINED PC_JACK_VERSION AND NOT PC_JACK_VERSION STREQUAL "")
  set(JACK_VERSION "${PC_JACK_VERSION}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  JACK
  REQUIRED_VARS JACK_LIBRARY JACK_INCLUDE_DIR
  VERSION_VAR JACK_VERSION
)

if(JACK_FOUND)
  set(JACK_LIBRARIES "${JACK_LIBRARY}")
  set(JACK_INCLUDE_DIRS "${JACK_INCLUDE_DIR}")
  set(JACK_DEFINITIONS ${PC_JACK_CFLAGS_OTHER})

  if(NOT TARGET JACK::jack)
    add_library(JACK::jack UNKNOWN IMPORTED)
    set_target_properties(
      JACK::jack
      PROPERTIES
        IMPORTED_LOCATION "${JACK_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_JACK_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${JACK_INCLUDE_DIR}"
    )
  endif()
endif()
