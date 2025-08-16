#[=======================================================================[.rst:
FindOpus
--------

Finds the Opus library.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``Opus_FOUND``
  True if the system has the Opus library.
``Opus_INCLUDE_DIRS``
  Include directories needed to use Opus.
``Opus_LIBRARIES``
  Libraries needed to link to Opus.
``Opus_DEFINITIONS``
  Compile definitions needed to use Opus.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``Opus_INCLUDE_DIR``
  The directory containing ``opus.h``.
``Opus_LIBRARY``
  The path to the Opus library.

#]=======================================================================]

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_Opus QUIET opus)
endif()

find_path(
  Opus_INCLUDE_DIR
  NAMES opus/opus.h
  HINTS ${PC_Opus_INCLUDE_DIRS}
  DOC "Opus include directory"
)
mark_as_advanced(Opus_INCLUDE_DIR)

find_library(
  Opus_LIBRARY
  NAMES opus
  HINTS ${PC_Opus_LIBRARY_DIRS}
  DOC "Opus library"
)
mark_as_advanced(Opus_LIBRARY)

if(DEFINED PC_Opus_VERSION AND NOT PC_Opus_VERSION STREQUAL "")
  set(Opus_VERSION "${PC_Opus_VERSION}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Opus
  REQUIRED_VARS Opus_LIBRARY Opus_INCLUDE_DIR
  VERSION_VAR Opus_VERSION
)

if(Opus_FOUND)
  set(Opus_LIBRARIES ${Opus_LIBRARY})
  set(Opus_INCLUDE_DIRS ${Opus_INCLUDE_DIR})
  set(Opus_DEFINITIONS ${PC_Opus_CFLAGS_OTHER})

  if(NOT TARGET Opus::Opus)
    add_library(Opus::Opus UNKNOWN IMPORTED)
    set_target_properties(
      Opus::Opus
      PROPERTIES
        IMPORTED_LOCATION "${Opus_LIBRARIES}"
        INTERFACE_COMPILE_OPTIONS "${Opus_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${Opus_INCLUDE_DIRS}"
    )
  endif()
endif()
