#[=======================================================================[.rst:
FindFFTW3f
----------

Finds the single precision FFTW3 library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``FFTW3::fftw3f``
  The single precision FFTW3 library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``FFTW3f_FOUND``
  True if the system has the single precision FFTW3 library.
``FFTW3f_INCLUDE_DIRS``
  Include directories needed to use FFTW3f.
``FFTW3f_LIBRARIES``
  Libraries needed to link to FFTW3f.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``FFTW3f_INCLUDE_DIR``
  The directory containing ``fftw3.h``.
``FFTW3f_LIBRARY``
  The path to the single precision FFTW3 library.

#]=======================================================================]

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_Fftw3f QUIET fftw3f)
endif()

find_path(
  FFTW3f_INCLUDE_DIR
  NAMES fftw3.h
  HINTS ${PC_Fftw3f_INCLUDE_DIRS}
  DOC "FFTW3 include directory"
)
mark_as_advanced(FFTW3f_INCLUDE_DIR)

find_library(
  FFTW3f_LIBRARY
  NAMES fftw3f fftw3f-3
  HINTS ${PC_Fftw3f_LIBRARY_DIRS}
  DOC "FFTW3f library"
)
mark_as_advanced(FFTW3f_LIBRARY)

if(DEFINED PC_Fftw3f_VERSION AND NOT PC_Fftw3f_VERSION STREQUAL "")
  set(FFTW3f_VERSION "${PC_Fftw3f_VERSION}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  FFTW3f
  REQUIRED_VARS FFTW3f_LIBRARY FFTW3f_INCLUDE_DIR
  VERSION_VAR FFTW3f_VERSION
)

if(FFTW3f_FOUND)
  set(FFTW3f_LIBRARIES "${FFTW3f_LIBRARY}")
  set(FFTW3f_INCLUDE_DIRS "${FFTW3f_INCLUDE_DIR}")
  set(FFTW3f_DEFINITIONS ${PC_Fftw3f_CFLAGS_OTHER})

  if(NOT TARGET FFTW3::fftw3f)
    add_library(FFTW3::fftw3f UNKNOWN IMPORTED)
    set_target_properties(
      FFTW3::fftw3f
      PROPERTIES
        IMPORTED_LOCATION "${FFTW3f_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_Fftw3f_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${FFTW3f_INCLUDE_DIR}"
    )
  endif()
endif()
