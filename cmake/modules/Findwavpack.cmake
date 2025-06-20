#[=======================================================================[.rst:
Findwavpack
-----------

Finds the wavpack library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``WavPack::wavpack``
  The WavPack library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``wavpack_FOUND``
  True if the system has the wavpack library.
``wavpack_INCLUDE_DIRS``
  Include directories needed to use wavpack.
``wavpack_LIBRARIES``
  Libraries needed to link to wavpack.
``wavpack_DEFINITIONS``
  Compile definitions needed to use wavpack.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``wavpack_INCLUDE_DIR``
  The directory containing ``wavpack.h``.
``wavpack_LIBRARY``
  The path to the wavpack library.

#]=======================================================================]

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_wavpack QUIET wavpack)
endif()

find_path(
  wavpack_INCLUDE_DIR
  NAMES wavpack.h
  PATH_SUFFIXES wavpack
  HINTS ${PC_wavpack_INCLUDE_DIRS}
  DOC "wavpack include directory"
)
mark_as_advanced(wavpack_INCLUDE_DIR)

find_library(
  wavpack_LIBRARY
  NAMES wavpack wv wavpackdll
  HINTS ${PC_wavpack_LIBRARY_DIRS}
  DOC "wavpack library"
)
mark_as_advanced(wavpack_LIBRARY)

if(DEFINED PC_wavpack_VERSION AND NOT PC_wavpack_VERSION STREQUAL "")
  set(wavpack_VERSION "${PC_wavpack_VERSION}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  wavpack
  REQUIRED_VARS wavpack_LIBRARY wavpack_INCLUDE_DIR
  VERSION_VAR wavpack_VERSION
)

if(wavpack_FOUND)
  set(wavpack_LIBRARIES "${wavpack_LIBRARY}")
  set(wavpack_INCLUDE_DIRS "${wavpack_INCLUDE_DIR}")
  set(wavpack_DEFINITIONS ${PC_wavpack_CFLAGS_OTHER})

  if(NOT TARGET WavPack::wavpack)
    add_library(WavPack::wavpack UNKNOWN IMPORTED)
    set_target_properties(
      WavPack::wavpack
      PROPERTIES
        IMPORTED_LOCATION "${wavpack_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_wavpack_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${wavpack_INCLUDE_DIR}"
    )
  endif()
endif()
