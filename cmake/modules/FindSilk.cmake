# This file is part of Mixxx, Digital DJ'ing software.
# Copyright (C) 2001-2020 Mixxx Development Team
# Distributed under the GNU General Public Licence (GPL) version 2 or any later
# later version. See the LICENSE file for details.

#[=======================================================================[.rst:
FindSilk
--------

Finds the Silk library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``Silk::Common``
  The Silk common library

``Silk::Fixed``
  The Silk fixed library

``Silk::Float``
  The Silk float library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``Silk_FOUND``
  True if the system has the Silk library.
``Silk_LIBRARIES``
  Libraries needed to link to Silk.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``Silk_Common_LIBRARY``
  The path to the Silk common library.

``Silk_Fixed_LIBRARY``
  The path to the Silk fixed library.

``Silk_Float_LIBRARY``
  The path to the Silk float library.

#]=======================================================================]

find_library(Silk_Common_LIBRARY
  NAMES silk_common
  DOC "Silk common library"
)
mark_as_advanced(Silk_Common_LIBRARY)

find_library(Silk_Fixed_LIBRARY
  NAMES silk_fixed
  DOC "Silk fixed library"
)
mark_as_advanced(Silk_Fixed_LIBRARY)

find_library(Silk_Float_LIBRARY
  NAMES silk_float
  DOC "Silk float library"
)
mark_as_advanced(Silk_Float_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Silk
  DEFAULT_MSG
  Silk_Common_LIBRARY
  Silk_Fixed_LIBRARY
  Silk_Float_LIBRARY
)

if(Silk_FOUND)
  set(Silk_LIBRARIES
    "${Silk_Common_LIBRARY}"
    "${Silk_Fixed_LIBRARY}"
    "${Silk_Float_LIBRARY}"
  )

  if(NOT TARGET Silk::Common)
    add_library(Silk::Common UNKNOWN IMPORTED)
    set_target_properties(Silk::Common
      PROPERTIES
        IMPORTED_LOCATION "${Silk_Common_LIBRARY}"
    )
  endif()

  if(NOT TARGET Silk::Fixed)
    add_library(Silk::Fixed UNKNOWN IMPORTED)
    set_target_properties(Silk::Fixed
      PROPERTIES
        IMPORTED_LOCATION "${Silk_Fixed_LIBRARY}"
        INTERFACE_LINK_LIBRARIES Silk::Common
    )
  endif()

  if(NOT TARGET Silk::Float)
    add_library(Silk::Float UNKNOWN IMPORTED)
    set_target_properties(Silk::Float
      PROPERTIES
        IMPORTED_LOCATION "${Silk_Float_LIBRARY}"
        INTERFACE_LINK_LIBRARIES Silk::Common
    )
  endif()
endif()
