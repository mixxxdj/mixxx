#[=======================================================================[.rst:
Findrubberband
--------------

Finds the rubberband library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``rubberband::rubberband``
  The rubberband library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``rubberband_FOUND``
  True if the system has the rubberband library.
``rubberband_INCLUDE_DIRS``
  Include directories needed to use rubberband.
``rubberband_LIBRARIES``
  Libraries needed to link to rubberband.
``rubberband_DEFINITIONS``
  Compile definitions needed to use rubberband.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``rubberband_INCLUDE_DIR``
  The directory containing ``rubberband/RubberBandStretcher.h``.
``rubberband_LIBRARY``
  The path to the rubberband library.

#]=======================================================================]

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_rubberband QUIET rubberband)
endif()

find_path(
  rubberband_INCLUDE_DIR
  NAMES rubberband/RubberBandStretcher.h
  HINTS ${PC_rubberband_INCLUDE_DIRS}
  DOC "rubberband include directory"
)
mark_as_advanced(rubberband_INCLUDE_DIR)

find_library(
  rubberband_LIBRARY
  NAMES rubberband rubberband-library rubberband-dll
  HINTS ${PC_rubberband_LIBRARY_DIRS}
  DOC "rubberband library"
)
mark_as_advanced(rubberband_LIBRARY)

if(DEFINED PC_rubberband_VERSION AND NOT PC_rubberband_VERSION STREQUAL "")
  set(rubberband_VERSION "${PC_rubberband_VERSION}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  rubberband
  REQUIRED_VARS rubberband_LIBRARY rubberband_INCLUDE_DIR
  VERSION_VAR rubberband_VERSION
)

if(rubberband_FOUND)
  set(rubberband_LIBRARIES "${rubberband_LIBRARY}")
  set(rubberband_INCLUDE_DIRS "${rubberband_INCLUDE_DIR}")
  set(rubberband_DEFINITIONS ${PC_rubberband_CFLAGS_OTHER})

  if(NOT TARGET rubberband::rubberband)
    add_library(rubberband::rubberband UNKNOWN IMPORTED)
    set_target_properties(
      rubberband::rubberband
      PROPERTIES
        IMPORTED_LOCATION "${rubberband_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_rubberband_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${rubberband_INCLUDE_DIR}"
    )
    is_static_library(rubberband_IS_STATIC rubberband::rubberband)
    if(rubberband_IS_STATIC)
      find_library(SAMPLERATE_LIBRARY samplerate REQUIRED)
      set_property(
        TARGET rubberband::rubberband
        APPEND
        PROPERTY INTERFACE_LINK_LIBRARIES ${SAMPLERATE_LIBRARY}
      )
      find_package(FFTW3)
      if(FFTW_FOUND)
        set_property(
          TARGET rubberband::rubberband
          APPEND
          PROPERTY INTERFACE_LINK_LIBRARIES FFTW3::fftw3
        )
      endif()
      find_package(Sleef)
      if(Sleef_FOUND)
        set_property(
          TARGET rubberband::rubberband
          APPEND
          PROPERTY INTERFACE_LINK_LIBRARIES Sleef::sleef Sleef::sleefdft
        )
      endif()
    endif()
  endif()
endif()
