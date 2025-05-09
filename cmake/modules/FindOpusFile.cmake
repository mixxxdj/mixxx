#[=======================================================================[.rst:
FindOpus
--------

Finds the Opus library.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``OpusFile_FOUND``
  True if the system has the Opus library.
``OpusFile_INCLUDE_DIRS``
  Include directories needed to use Opus.
``OpusFile_LIBRARIES``
  Libraries needed to link to Opus.
``OpusFile_DEFINITIONS``
  Compile definitions needed to use Opus.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``OpusFile_INCLUDE_DIR``
  The directory containing ``opus.h``.
``OpusFile_LIBRARY``
  The path to the Opus library.
``OpusFile_INCLUDE_DIR``
  The directory containing ``opusfile.h``.
``OpusFile_LIBRARY``
  The path to the Opus library.

#]=======================================================================]

include(IsStaticLibrary)

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_OpusFile QUIET opusfile)
endif()

find_path(
  OpusFile_INCLUDE_DIR
  NAMES opusfile.h
  PATH_SUFFIXES opus
  HINTS ${PC_OpusFile_INCLUDE_DIRS}
  DOC "Opusfile include directory"
)
mark_as_advanced(OpusFile_INCLUDE_DIR)

find_library(
  OpusFile_LIBRARY
  NAMES opusfile
  HINTS ${PC_OpusFile_LIBRARY_DIRS}
  DOC "Opusfile library"
)
mark_as_advanced(OpusFile_LIBRARY)

if(DEFINED PC_OpusFile_VERSION AND NOT PC_OpusFile_VERSION STREQUAL "")
  set(OpusFile_VERSION "${PC_OpusFile_VERSION}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  OpusFile
  REQUIRED_VARS OpusFile_LIBRARY OpusFile_INCLUDE_DIR
  VERSION_VAR OpusFile_VERSION
)

if(OpusFile_FOUND)
  set(OpusFile_LIBRARIES ${OpusFile_LIBRARY})
  set(OpusFile_INCLUDE_DIRS ${OpusFile_INCLUDE_DIR})
  set(OpusFile_DEFINITIONS ${PC_OpusFile_CFLAGS_OTHER})

  if(NOT TARGET OpusFile::OpusFile)
    add_library(OpusFile::OpusFile UNKNOWN IMPORTED)
    set_target_properties(
      OpusFile::OpusFile
      PROPERTIES
        IMPORTED_LOCATION "${OpusFile_LIBRARIES}"
        INTERFACE_COMPILE_OPTIONS "${OpusFile_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${OpusFile_INCLUDE_DIRS}"
    )
  endif()
endif()
