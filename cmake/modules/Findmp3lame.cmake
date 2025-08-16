#[=======================================================================[.rst:
Findmp3lame
-----------

Finds the LAME library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``mp3lame::mp3lame``
  The LAME library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``mp3lame_FOUND``
  True if the system has the LAME library.
``mp3lame_INCLUDE_DIRS``
  Include directories needed to use LAME.
``mp3lame_LIBRARIES``
  Libraries needed to link to LAME.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``mp3lame_INCLUDE_DIR``
  The directory containing ``lame/lame.h``.
``mp3lame_LIBRARY``
  The path to the LAME library.

#]=======================================================================]

find_path(mp3lame_INCLUDE_DIR NAMES lame/lame.h DOC "LAME include directory")
mark_as_advanced(mp3lame_INCLUDE_DIR)

find_library(mp3lame_LIBRARY NAMES mp3lame mp3lame-static DOC "LAME library")
mark_as_advanced(mp3lame_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  mp3lame
  DEFAULT_MSG
  mp3lame_LIBRARY
  mp3lame_INCLUDE_DIR
)

if(mp3lame_FOUND)
  set(mp3lame_LIBRARIES "${mp3lame_LIBRARY}")
  set(mp3lame_INCLUDE_DIRS "${mp3lame_INCLUDE_DIR}")

  if(NOT TARGET mp3lame::mp3lame)
    add_library(mp3lame::mp3lame UNKNOWN IMPORTED)
    set_target_properties(
      mp3lame::mp3lame
      PROPERTIES
        IMPORTED_LOCATION "${mp3lame_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${mp3lame_INCLUDE_DIR}"
    )
  endif()
endif()
