#[=======================================================================[.rst:
Findlibsamplerate
--------------

Finds the SampleRate library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``libsamplerate::libsamplerate``
  The Libsamplerate library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``samplerate_FOUND``
  True if the system has the libsamplerate library.
``samplerate_INCLUDE_DIRS``
  Include directories needed to use libsamplerate.
``samplerate_LIBRARIES``
  Libraries needed to link to libsamplerate.
``samplerate_DEFINITIONS``
  Compile definitions needed to use libsamplerate.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``libsamplerate_INCLUDE_DIR``
  The directory containing ``libsamplerate/<libsamplerate header>``.
``libsamplerate_LIBRARY``
  The path to the libsamplerate library.

#]=======================================================================]
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_libsamplerate QUIET samplerate) # .pc exists, /usr/lib/x86_64-linux-gnu/pkgconfig/samplerate.pc
endif()

# if .pc not present
find_path(
  libsamplerate_INCLUDE_DIR
  NAMES samplerate.h
  HINTS ${PC_libsamplerate_INCLUDE_DIRS}
  PATH_SUFFIXES samplerate
  DOC "libsamplerate include directory"
)
mark_as_advanced(libsamplerate_INCLUDE_DIR) # cache

find_library(
  libsamplerate_LIBRARY
  NAMES samplerate samplerate-dev
  HINTS ${PC_libsamplerate_LIBRARY_DIRS}
  DOC "libsamplerate library"
)
mark_as_advanced(libsamplerate_LIBRARY)

# in case a libsamplerate version is specified, not needed rn
if(
  DEFINED PC_libsamplerate_VERSION
  AND NOT PC_libsamplerate_VERSION STREQUAL ""
)
  set(libsamplerate_VERSION "${PC_libsamplerate_VERSION}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  samplerate
  REQUIRED_VARS libsamplerate_LIBRARY libsamplerate_INCLUDE_DIR
  VERSION_VAR libsamplerate_VERSION
)

# export target name
if(samplerate_FOUND)
  set(samplerate_LIBRARIES "${libsamplerate_LIBRARY}")
  set(samplerate_INCLUDE_DIRS "${libsamplerate_INCLUDE_DIR}")
  set(samplerate_DEFINITIONS ${PC_libsamplerate_CFLAGS_OTHER})

  if(NOT TARGET libsamplerate::libsamplerate)
    add_library(libsamplerate::libsamplerate UNKNOWN IMPORTED)
    set_target_properties(
      libsamplerate::libsamplerate
      PROPERTIES
        IMPORTED_LOCATION "${libsamplerate_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_libsamplerate_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${libsamplerate_INCLUDE_DIR}"
    )
  endif()
endif()
