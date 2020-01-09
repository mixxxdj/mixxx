# This file is part of Mixxx, Digital DJ'ing software.
# Copyright (C) 2001-2020 Mixxx Development Team
# Distributed under the GNU General Public Licence (GPL) version 2 or any later
# later version. See the LICENSE file for details.

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

find_path(PortMidi_INCLUDE_DIR
  NAMES portmidi.h
  PATH_SUFFIXES portmidi
  DOC "PortMidi include directory")
mark_as_advanced(PortMidi_INCLUDE_DIR)

find_path(PortTime_INCLUDE_DIR
  NAMES porttime.h
  PATH_SUFFIXES portmidi porttime
  DOC "PortTime include directory")
mark_as_advanced(PortTime_INCLUDE_DIR)

find_library(PortMidi_LIBRARY
  NAMES portmidi portmidi_s
  DOC "PortMidi library"
)
mark_as_advanced(PortMidi_LIBRARY)

find_library(PortTime_LIBRARY
  NAMES porttime
  DOC "PortTime library"
)
mark_as_advanced(PortTime_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  PortMidi
  DEFAULT_MSG
  PortMidi_LIBRARY
  PortMidi_INCLUDE_DIR
  PortTime_INCLUDE_DIR
)

if(PortMidi_FOUND)
  set(PortMidi_LIBRARIES ${PortMidi_LIBRARY})
  # Depending on the library configuration PortTime might be statically
  # linked with PortMidi.
  if(PortTime_LIBRARY)
    list(APPEND PortMidi_LIBRARIES ${PortTime_LIBRARY})
  endif()
  set(PortMidi_INCLUDE_DIRS ${PortMidi_INCLUDE_DIR} ${PortTime_INCLUDE_DIR})
endif()
