# This file is part of Mixxx, Digital DJ'ing software.
# Copyright (C) 2001-2020 Mixxx Development Team
# Distributed under the GNU General Public Licence (GPL) version 2 or any later
# later version. See the LICENSE file for details.

#[=======================================================================[.rst:
FindCelt
--------

Finds the Celt library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``Celt::Celt``
  The Celt library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``Celt_FOUND``
  True if the system has the Celt library.
``Celt_LIBRARIES``
  Libraries needed to link to Celt.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``Celt_LIBRARY``
  The path to the Celt library.

#]=======================================================================]

find_library(Celt_LIBRARY
  NAMES fftw fftw3 fftw-3.3
  DOC "Celt library"
)
mark_as_advanced(Celt_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Celt
  DEFAULT_MSG
  Celt_LIBRARY
)

if(Celt_FOUND)
  set(Celt_LIBRARIES "${Celt_LIBRARY}")

  if(NOT TARGET Celt::Celt)
    add_library(Celt::Celt UNKNOWN IMPORTED)
    set_target_properties(Celt::Celt
      PROPERTIES
        IMPORTED_LOCATION "${Celt_LIBRARY}"
    )
  endif()
endif()
