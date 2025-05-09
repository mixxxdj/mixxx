#[=======================================================================[.rst:
FindPortAudio
--------

Finds the PortAudio library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``PortAudio::PortAudio``
  The PortAudio library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``PortAudio_FOUND``
  True if the system has the PortAudio library.
``PortAudio_INCLUDE_DIRS``
  Include directories needed to use PortAudio.
``PortAudio_LIBRARIES``
  Libraries needed to link to PortAudio.
``PortAudio_DEFINITIONS``
  Compile definitions needed to use PortAudio.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``PortAudio_INCLUDE_DIR``
  The directory containing ``PortAudio/all.h``.
``PortAudio_LIBRARY``
  The path to the PortAudio library.

#]=======================================================================]

include(IsStaticLibrary)

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_PortAudio QUIET portaudio-2.0)
endif()

find_path(
  PortAudio_INCLUDE_DIR
  NAMES portaudio.h
  HINTS ${PC_PortAudio_INCLUDE_DIRS}
  DOC "PortAudio include directory"
)
mark_as_advanced(PortAudio_INCLUDE_DIR)

# Temporary hack until https://github.com/PortAudio/portaudio/pull/635 is released.
find_path(
  PortAudio_ALSA_H
  NAMES pa_linux_alsa.h
  HINTS ${PC_PortAudio_INCLUDE_DIRS}
)
mark_as_advanced(PortAudio_ALSA_H)

find_library(
  PortAudio_LIBRARY
  NAMES portaudio
  HINTS ${PC_PortAudio_LIBRARY_DIRS}
  DOC "PortAudio library"
)
mark_as_advanced(PortAudio_LIBRARY)

if(DEFINED PC_PortAudio_VERSION AND NOT PC_PortAudio_VERSION STREQUAL "")
  set(PortAudio_VERSION "${PC_PortAudio_VERSION}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  PortAudio
  REQUIRED_VARS PortAudio_LIBRARY PortAudio_INCLUDE_DIR
  VERSION_VAR PortAudio_VERSION
)

if(PortAudio_FOUND)
  if(NOT TARGET PortAudio::PortAudio)
    add_library(PortAudio::PortAudio UNKNOWN IMPORTED)
    set_target_properties(
      PortAudio::PortAudio
      PROPERTIES
        IMPORTED_LOCATION "${PortAudio_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_PortAudio_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${PortAudio_INCLUDE_DIR}"
    )
    is_static_library(PortAudio_IS_STATIC PortAudio::PortAudio)
    if(PortAudio_IS_STATIC)
      if(PortAudio_ALSA_H)
        find_package(ALSA)
        if(ALSA_FOUND)
          set_property(
            TARGET PortAudio::PortAudio
            APPEND
            PROPERTY INTERFACE_LINK_LIBRARIES ALSA::ALSA
          )
        endif()
      endif()
      find_package(JACK)
      if(JACK_FOUND)
        set_property(
          TARGET PortAudio::PortAudio
          APPEND
          PROPERTY INTERFACE_LINK_LIBRARIES JACK::jack
        )
      endif()
    endif()
    if(PortAudio_ALSA_H)
      target_compile_definitions(PortAudio::PortAudio INTERFACE PA_USE_ALSA)
    endif()
  endif()
endif()
