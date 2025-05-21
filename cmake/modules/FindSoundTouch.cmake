#[=======================================================================[.rst:
FindSoundTouch
--------------

Finds the SoundTouch library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``SoundTouch::SoundTouch``
  The SoundTouch library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``SoundTouch_FOUND``
  True if the system has the SoundTouch library.
``SoundTouch_INCLUDE_DIRS``
  Include directories needed to use SoundTouch.
``SoundTouch_LIBRARIES``
  Libraries needed to link to SoundTouch.
``SoundTouch_DEFINITIONS``
  Compile definitions needed to use SoundTouch.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``SoundTouch_INCLUDE_DIR``
  The directory containing ``soundtouch/SoundTouch.h``.
``SoundTouch_LIBRARY``
  The path to the SoundTouch library.

#]=======================================================================]

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_SoundTouch QUIET soundtouch)
endif()

find_path(
  SoundTouch_INCLUDE_DIR
  NAMES soundtouch/SoundTouch.h
  HINTS ${PC_SoundTouch_INCLUDE_DIRS}
  DOC "SoundTouch include directory"
)
mark_as_advanced(SoundTouch_INCLUDE_DIR)

find_library(
  SoundTouch_LIBRARY
  NAMES SoundTouch
  HINTS ${PC_SoundTouch_LIBRARY_DIRS}
  DOC "SoundTouch library"
)
mark_as_advanced(SoundTouch_LIBRARY)

# Version detection
if(DEFINED PC_SoundTouch_VERSION AND NOT PC_SoundTouch_VERSION STREQUAL "")
  set(SoundTouch_VERSION "${PC_SoundTouch_VERSION}")
else()
  if(EXISTS "${SoundTouch_INCLUDE_DIR}/soundtouch/SoundTouch.h")
    file(
      READ
      "${SoundTouch_INCLUDE_DIR}/soundtouch/SoundTouch.h"
      SoundTouch_H_CONTENTS
    )
    string(
      REGEX MATCH
      "#define SOUNDTOUCH_VERSION[ \t]+\"([0-9]+\.[0-9]+\.[0-9]+)\""
      _dummy
      "${SoundTouch_H_CONTENTS}"
    )
    set(SoundTouch_VERSION "${CMAKE_MATCH_1}")
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  SoundTouch
  REQUIRED_VARS SoundTouch_LIBRARY SoundTouch_INCLUDE_DIR
  VERSION_VAR SoundTouch_VERSION
)

if(SoundTouch_FOUND)
  set(SoundTouch_LIBRARIES "${SoundTouch_LIBRARY}")
  set(SoundTouch_INCLUDE_DIRS "${SoundTouch_INCLUDE_DIR}")
  set(SoundTouch_DEFINITIONS ${PC_SoundTouch_CFLAGS_OTHER})

  if(NOT TARGET SoundTouch::SoundTouch)
    add_library(SoundTouch::SoundTouch UNKNOWN IMPORTED)
    set_target_properties(
      SoundTouch::SoundTouch
      PROPERTIES
        IMPORTED_LOCATION "${SoundTouch_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_SoundTouch_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${SoundTouch_INCLUDE_DIR}"
    )
  endif()
endif()
