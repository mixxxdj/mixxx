#[=======================================================================[.rst:
FindTagLib
-----------

Finds the TagLib library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``TagLib::TagLib``
  The TagLib library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``TagLib_FOUND``
  True if the system has the TagLib library.
``TagLib_INCLUDE_DIRS``
  Include directories needed to use TagLib.
``TagLib_LIBRARIES``
  Libraries needed to link to TagLib.
``TagLib_DEFINITIONS``
  Compile definitions needed to use TagLib.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``TagLib_INCLUDE_DIR``
  The directory containing ``ebur128.h``.
``TagLib_LIBRARY``
  The path to the TagLib library.

#]=======================================================================]

include(IsStaticLibrary)

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  if(UNIX AND NOT APPLE)
    # prioritize the taglib1 package introduced in https://www.archlinux.de/packages/extra/x86_64/taglib1
    set(
      ENV{PKG_CONFIG_PATH}
      "/usr/lib/taglib1/pkgconfig/:$ENV{PKG_CONFIG_PATH}"
    )
  endif()
  pkg_check_modules(PC_TagLib QUIET taglib)
endif()

find_path(
  TagLib_INCLUDE_DIR
  NAMES tag.h
  HINTS ${PC_TagLib_INCLUDE_DIRS}
  PATH_SUFFIXES taglib
  DOC "TagLib include directory"
)
mark_as_advanced(TagLib_INCLUDE_DIR)

find_library(
  TagLib_LIBRARY
  NAMES tag
  HINTS ${PC_TagLib_LIBRARY_DIRS}
  DOC "TagLib library"
)
mark_as_advanced(TagLib_LIBRARY)

if(DEFINED PC_TagLib_VERSION AND NOT PC_TagLib_VERSION STREQUAL "")
  set(TagLib_VERSION "${PC_TagLib_VERSION}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  TagLib
  REQUIRED_VARS TagLib_LIBRARY TagLib_INCLUDE_DIR
  VERSION_VAR TagLib_VERSION
)

# Version detection
if(DEFINED PC_TagLib_VERSION AND NOT PC_TagLib_VERSION STREQUAL "")
  set(TagLib_VERSION "${PC_TagLib_VERSION}")
else()
  if(EXISTS "${TagLib_INCLUDE_DIR}/taglib.h")
    file(READ "${TagLib_INCLUDE_DIR}/taglib.h" TagLib_H_CONTENTS)
    string(
      REGEX MATCH
      "#define TAGLIB_MAJOR_VERSION ([0-9]+)"
      _dummy
      "${TagLib_H_CONTENTS}"
    )
    set(TagLib_VERSION_MAJOR "${CMAKE_MATCH_1}")
    string(
      REGEX MATCH
      "#define TAGLIB_MINOR_VERSION ([0-9]+)"
      _dummy
      "${TagLib_H_CONTENTS}"
    )
    set(TagLib_VERSION_MINOR "${CMAKE_MATCH_1}")
    string(
      REGEX MATCH
      "#define TAGLIB_PATCH_VERSION ([0-9]+)"
      _dummy
      "${TagLib_H_CONTENTS}"
    )
    set(TagLib_VERSION_PATCH "${CMAKE_MATCH_1}")
    # Simply using if(NOT) does not work because 0 is a valid value, so compare to empty string.
    if(
      NOT TagLib_VERSION_MAJOR STREQUAL ""
      AND NOT TagLib_VERSION_MINOR STREQUAL ""
      AND NOT TagLib_VERSION_PATCH STREQUAL ""
    )
      set(
        TagLib_VERSION
        "${TagLib_VERSION_MAJOR}.${TagLib_VERSION_MINOR}.${TagLib_VERSION_PATCH}"
      )
    endif()
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  TagLib
  REQUIRED_VARS TagLib_LIBRARY TagLib_INCLUDE_DIR
  VERSION_VAR TagLib_VERSION
)

if(TagLib_FOUND)
  set(TagLib_LIBRARIES "${TagLib_LIBRARY}")
  set(TagLib_INCLUDE_DIRS "${TagLib_INCLUDE_DIR}")
  set(TagLib_DEFINITIONS ${PC_TagLib_CFLAGS_OTHER})

  if(NOT TARGET TagLib::TagLib)
    add_library(TagLib::TagLib UNKNOWN IMPORTED)
    set_target_properties(
      TagLib::TagLib
      PROPERTIES
        IMPORTED_LOCATION "${TagLib_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_TagLib_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${TagLib_INCLUDE_DIR}"
    )
    is_static_library(Taglib_IS_STATIC TagLib::TagLib)
    if(Taglib_IS_STATIC)
      if(WIN32)
        set_property(
          TARGET TagLib::TagLib
          APPEND
          PROPERTY INTERFACE_COMPILE_DEFINITIONS TAGLIB_STATIC
        )
      endif()
    endif()
  endif()
endif()
