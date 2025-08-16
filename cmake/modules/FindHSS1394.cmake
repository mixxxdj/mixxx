#[=======================================================================[.rst:
FindHSS1394
-----------

Finds the HSS1394 library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``HSS1394::HSS1394``
  The HSS1304 library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``HSS1394_FOUND``
  True if the system has the HSS1394 library.
``HSS1394_INCLUDE_DIRS``
  Include directories needed to use HSS1394.
``HSS1394_LIBRARIES``
  Libraries needed to link to HSS1394.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``HSS1394_INCLUDE_DIR``
  The directory containing ``HSS1394/HSS1394.h``.
``HSS1394_LIBRARY``
  The path to the HSS1394 library.

#]=======================================================================]

find_path(
  HSS1394_INCLUDE_DIR
  NAMES HSS1394/HSS1394.h
  DOC "HSS1394 include directory"
)
mark_as_advanced(HSS1394_INCLUDE_DIR)

find_library(HSS1394_LIBRARY NAMES hss1394 DOC "HSS1394 library")
mark_as_advanced(HSS1394_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  HSS1394
  DEFAULT_MSG
  HSS1394_LIBRARY
  HSS1394_INCLUDE_DIR
)

if(HSS1394_FOUND)
  set(HSS1394_LIBRARIES "${HSS1394_LIBRARY}")
  set(HSS1394_INCLUDE_DIRS "${HSS1394_INCLUDE_DIR}")

  if(NOT TARGET HSS1394::HSS1394)
    add_library(HSS1394::HSS1394 UNKNOWN IMPORTED)
    set_target_properties(
      HSS1394::HSS1394
      PROPERTIES
        IMPORTED_LOCATION "${HSS1394_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${HSS1394_INCLUDE_DIR}"
    )
  endif()
endif()
