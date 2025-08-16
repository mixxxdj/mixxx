#[=======================================================================[.rst:
FindPortMidi
---------------

Finds the PortMidi library.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``PortMidi_FOUND``
  True if the system has the PortMidi library.
``PortMidi_INCLUDE_DIRS``
  Include directories needed to use PortMidi.
``PortMidi_LIBRARIES``
  Libraries needed to link to PortMidi.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``PortMidi_INCLUDE_DIR``
  The directory containing ``portmidi.h``.
``PortTime_INCLUDE_DIR``
  The directory containing ``porttime.h``.
``PortMidi_LIBRARY``
  The path to the PortMidi library.
``PortTime_LIBRARY``
  The path to the PortTime library.

#]=======================================================================]

include(IsStaticLibrary)

find_path(
  PortMidi_INCLUDE_DIR
  NAMES portmidi.h
  PATH_SUFFIXES portmidi
  DOC "PortMidi include directory"
)
mark_as_advanced(PortMidi_INCLUDE_DIR)

find_path(
  PortTime_INCLUDE_DIR
  NAMES porttime.h
  PATH_SUFFIXES portmidi porttime
  DOC "PortTime include directory"
)
mark_as_advanced(PortTime_INCLUDE_DIR)

find_library(PortMidi_LIBRARY NAMES portmidi portmidi_s DOC "PortMidi library")
mark_as_advanced(PortMidi_LIBRARY)

find_library(PortTime_LIBRARY NAMES porttime DOC "PortTime library")
mark_as_advanced(PortTime_LIBRARY)

if(DEFINED PC_PortMidi_VERSION AND NOT PC_PortMidi_VERSION STREQUAL "")
  set(PortMidi_VERSION "${PC_PortMidi_VERSION}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  PortMidi
  REQUIRED_VARS PortMidi_LIBRARY PortMidi_INCLUDE_DIR PortTime_INCLUDE_DIR
  VERSION_VAR PortMidi_VERSION
)

if(PortMidi_FOUND AND NOT TARGET PortMidi::portmidi)
  add_library(PortMidi::portmidi UNKNOWN IMPORTED)
  set_target_properties(
    PortMidi::portmidi
    PROPERTIES
      IMPORTED_LOCATION "${PortMidi_LIBRARY}"
      INTERFACE_COMPILE_OPTIONS "${PC_PortMidi_CFLAGS_OTHER}"
      INTERFACE_INCLUDE_DIRECTORIES "${PortMidi_INCLUDE_DIR}"
  )

  set(PortMidi_LIBRARIES PortMidi::portmidi)
  # Depending on the library configuration PortTime might be statically
  # linked with PortMidi.
  if(PortTime_LIBRARY)
    if(NOT TARGET PortTime::porttime)
      add_library(PortTime::porttime UNKNOWN IMPORTED)
      set_target_properties(
        PortTime::porttime
        PROPERTIES
          IMPORTED_LOCATION "${PortTime_LIBRARY}"
          INTERFACE_COMPILE_OPTIONS "${PC_PortTime_CFLAGS_OTHER}"
          INTERFACE_INCLUDE_DIRECTORIES "${PortTime_INCLUDE_DIR}"
      )
    endif()
    list(APPEND PortMidi_LIBRARIES PortTime::porttime)
  endif()
  set(PortMidi_INCLUDE_DIRS ${PortMidi_INCLUDE_DIR} ${PortTime_INCLUDE_DIR})

  is_static_library(PortMidi_IS_STATIC PortMidi::portmidi)
  if(PortMidi_IS_STATIC)
    find_package(ALSA)
    if(ALSA_FOUND)
      set_property(
        TARGET PortMidi::portmidi
        APPEND
        PROPERTY INTERFACE_LINK_LIBRARIES ALSA::ALSA
      )
    endif()
  endif()
endif()
