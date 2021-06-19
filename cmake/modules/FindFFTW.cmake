# This file is part of Mixxx, Digital DJ'ing software.
# Copyright (C) 2001-2020 Mixxx Development Team
# Distributed under the GNU General Public Licence (GPL) version 2 or any later
# later version. See the LICENSE file for details.

#[=======================================================================[.rst:
FindFFTW
--------

Finds the FFTW library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``FFTW::FFTW``
  The FFTW library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``FFTW_FOUND``
  True if the system has the FFTW library.
``FFTW_INCLUDE_DIRS``
  Include directories needed to use FFTW.
``FFTW_LIBRARIES``
  Libraries needed to link to FFTW.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``FFTW_INCLUDE_DIR``
  The directory containing ``fftw3.h``.
``FFTW_LIBRARY``
  The path to the FFTW library.

#]=======================================================================]

find_path(FFTW_INCLUDE_DIR
  NAMES fftw3.h
  DOC "FFTW include directory")
mark_as_advanced(FFTW_INCLUDE_DIR)

find_library(FFTW_LIBRARY
  NAMES fftw fftw3 fftw-3.3
  DOC "FFTW library"
)
mark_as_advanced(FFTW_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  FFTW
  DEFAULT_MSG
  FFTW_LIBRARY
  FFTW_INCLUDE_DIR
)

if(FFTW_FOUND)
  set(FFTW_LIBRARIES "${FFTW_LIBRARY}")
  set(FFTW_INCLUDE_DIRS "${FFTW_INCLUDE_DIR}")

  if(NOT TARGET FFTW::FFTW)
    add_library(FFTW::FFTW UNKNOWN IMPORTED)
    set_target_properties(FFTW::FFTW
      PROPERTIES
        IMPORTED_LOCATION "${FFTW_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${FFTW_INCLUDE_DIR}"
    )
  endif()
endif()
