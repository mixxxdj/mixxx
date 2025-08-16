#[=======================================================================[.rst:
FindG72X
--------

Finds the G72X library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``G72X::G72X``
  The G72X library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``G72X_FOUND``
  True if the system has the G72X library.
``G72X_LIBRARIES``
  Libraries needed to link to G72X.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``G72X_LIBRARY``
  The path to the G72X library.

#]=======================================================================]

find_library(G72X_LIBRARY NAMES g72x DOC "G72X library")
mark_as_advanced(G72X_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(G72X DEFAULT_MSG G72X_LIBRARY)

if(G72X_FOUND)
  set(G72X_LIBRARIES "${G72X_LIBRARY}")

  if(NOT TARGET G72X::G72X)
    add_library(G72X::G72X UNKNOWN IMPORTED)
    set_target_properties(
      G72X::G72X
      PROPERTIES IMPORTED_LOCATION "${G72X_LIBRARY}"
    )
  endif()
endif()
