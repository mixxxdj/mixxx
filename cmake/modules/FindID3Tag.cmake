#[=======================================================================[.rst:
FindID3Tag
----------

Finds the ID3Tag library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``ID3Tag::ID3Tag``
  The ID3Tag library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``ID3Tag_FOUND``
  True if the system has the ID3Tag library.
``ID3Tag_INCLUDE_DIRS``
  Include directories needed to use ID3Tag.
``ID3Tag_LIBRARIES``
  Libraries needed to link to ID3Tag.
``ID3Tag_DEFINITIONS``
  Compile definitions needed to use ID3Tag.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``ID3Tag_INCLUDE_DIR``
  The directory containing ``id3tag.h``.
``ID3Tag_LIBRARY``
  The path to the ID3Tag library.

#]=======================================================================]

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_ID3Tag QUIET id3tag)
endif()

find_path(
  ID3Tag_INCLUDE_DIR
  NAMES id3tag.h
  HINTS ${PC_ID3Tag_INCLUDE_DIRS}
  PATH_SUFFIXES id3tag
  DOC "ID3Tag include directory"
)
mark_as_advanced(ID3Tag_INCLUDE_DIR)

find_library(
  ID3Tag_LIBRARY
  NAMES id3tag
  HINTS ${PC_ID3Tag_LIBRARY_DIRS}
  DOC "ID3Tag library"
)
mark_as_advanced(ID3Tag_LIBRARY)

if(DEFINED PC_ID3Tag_VERSION AND NOT PC_ID3Tag_VERSION STREQUAL "")
  set(ID3Tag_VERSION "${PC_ID3Tag_VERSION}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  ID3Tag
  REQUIRED_VARS ID3Tag_LIBRARY ID3Tag_INCLUDE_DIR
  VERSION_VAR ID3Tag_VERSION
)

if(ID3Tag_FOUND)
  set(ID3Tag_LIBRARIES "${ID3Tag_LIBRARY}")
  set(ID3Tag_INCLUDE_DIRS "${ID3Tag_INCLUDE_DIR}")
  set(ID3Tag_DEFINITIONS ${PC_ID3Tag_CFLAGS_OTHER})

  if(NOT TARGET ID3Tag::ID3Tag)
    add_library(ID3Tag::ID3Tag UNKNOWN IMPORTED)
    set_target_properties(
      ID3Tag::ID3Tag
      PROPERTIES
        IMPORTED_LOCATION "${ID3Tag_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_ID3Tag_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${ID3Tag_INCLUDE_DIR}"
    )
  endif()
endif()
