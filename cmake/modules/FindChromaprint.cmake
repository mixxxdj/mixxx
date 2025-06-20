#[=======================================================================[.rst:
FindChromaprint
---------------

Finds the Chromaprint library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``Chromaprint::Chromaprint``
  The Chromaprint library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``Chromaprint_FOUND``
  True if the system has the Chromaprint library.
``Chromaprint_INCLUDE_DIRS``
  Include directories needed to use Chromaprint.
``Chromaprint_LIBRARIES``
  Libraries needed to link to Chromaprint.
``Chromaprint_DEFINITIONS``
  Compile definitions needed to use Chromaprint.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``Chromaprint_INCLUDE_DIR``
  The directory containing ``chromaprint.h``.
``Chromaprint_LIBRARY``
  The path to the Chromaprint library.

#]=======================================================================]

include(IsStaticLibrary)

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_Chromaprint QUIET libchromaprint)
endif()

find_path(
  Chromaprint_INCLUDE_DIR
  NAMES chromaprint.h
  HINTS ${PC_Chromaprint_INCLUDE_DIRS}
  PATH_SUFFIXES chromaprint
  DOC "Chromaprint include directory"
)
mark_as_advanced(Chromaprint_INCLUDE_DIR)

find_library(
  Chromaprint_LIBRARY
  NAMES chromaprint chromaprint_p
  HINTS ${PC_Chromaprint_LIBRARY_DIRS}
  DOC "Chromaprint library"
)
mark_as_advanced(Chromaprint_LIBRARY)

if(DEFINED PC_Chromaprint_VERSION AND NOT PC_Chromaprint_VERSION STREQUAL "")
  set(Chromaprint_VERSION "${PC_Chromaprint_VERSION}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Chromaprint
  REQUIRED_VARS Chromaprint_LIBRARY Chromaprint_INCLUDE_DIR
  VERSION_VAR Chromaprint_VERSION
)

if(Chromaprint_FOUND)
  set(Chromaprint_LIBRARIES "${Chromaprint_LIBRARY}")
  set(Chromaprint_INCLUDE_DIRS "${Chromaprint_INCLUDE_DIR}")
  set(Chromaprint_DEFINITIONS ${PC_Chromaprint_CFLAGS_OTHER})

  if(NOT TARGET Chromaprint::Chromaprint)
    add_library(Chromaprint::Chromaprint UNKNOWN IMPORTED)
    set_target_properties(
      Chromaprint::Chromaprint
      PROPERTIES
        IMPORTED_LOCATION "${Chromaprint_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_Chromaprint_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${Chromaprint_INCLUDE_DIR}"
    )
    is_static_library(Chromaprint_IS_STATIC Chromaprint::Chromaprint)
    if(Chromaprint_IS_STATIC)
      if(WIN32)
        # used in chomaprint.h to set dllexport for Windows
        set_property(
          TARGET Chromaprint::Chromaprint
          APPEND
          PROPERTY INTERFACE_COMPILE_DEFINITIONS CHROMAPRINT_NODLL
        )
      endif()
      find_package(FFTW3 REQUIRED)
      set_property(
        TARGET Chromaprint::Chromaprint
        APPEND
        PROPERTY INTERFACE_LINK_LIBRARIES FFTW3::fftw3
      )
    endif()
  endif()
endif()
