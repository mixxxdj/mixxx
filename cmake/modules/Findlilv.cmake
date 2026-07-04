#[=======================================================================[.rst:
Findlilv
--------

Finds the lilv library and lv2-dev package containing 'units' headers.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``lilv::lilv``
  The lilv library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``lilv_FOUND``
  True if the system has the lilv library.
``lilv_INCLUDE_DIRS``
  Include directories needed to use lilv.
``lilv_LIBRARIES``
  Libraries needed to link to lilv.
``lilv_DEFINITIONS``
  Compile definitions needed to use lilv.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``lilv_INCLUDE_DIR``
  The directory containing ``lilv-0/lilv/lilv.h``.
``lilv_LIBRARY``
  The path to the lilv library.

#]=======================================================================]

include(IsStaticLibrary)

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_lilv QUIET lilv-0 lv2)
endif()

find_path(
  lilv_INCLUDE_DIR
  NAMES lilv/lilv.h
  PATH_SUFFIXES lilv-0
  HINTS ${PC_lilv_INCLUDE_DIRS}
  DOC "lilv include directory"
)
mark_as_advanced(lilv_INCLUDE_DIR)

find_library(
  lilv_LIBRARY
  NAMES lilv-0 lilv
  HINTS ${PC_lilv_LIBRARY_DIRS}
  DOC "lilv library"
)
mark_as_advanced(lilv_LIBRARY)

if(DEFINED PC_lilv_VERSION AND NOT PC_lilv_VERSION STREQUAL "")
  set(lilv_VERSION "${PC_lilv_VERSION}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  lilv
  REQUIRED_VARS lilv_LIBRARY lilv_INCLUDE_DIR
  VERSION_VAR lilv_VERSION
)

if(lilv_FOUND)
  set(lilv_LIBRARIES "${lilv_LIBRARY}")
  set(lilv_INCLUDE_DIRS "${lilv_INCLUDE_DIR}")
  set(lilv_DEFINITIONS ${PC_lilv_CFLAGS_OTHER})

  if(NOT TARGET lilv::lilv)
    add_library(lilv::lilv UNKNOWN IMPORTED)
    set_target_properties(
      lilv::lilv
      PROPERTIES
        IMPORTED_LOCATION "${lilv_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_lilv_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${lilv_INCLUDE_DIR}"
    )
    is_static_library(lilv_IS_STATIC lilv::lilv)
    if(lilv_IS_STATIC)
      find_package(sord CONFIG)
      if(NOT sord_FOUND)
        find_package(sord REQUIRED)
      endif()
      set_property(
        TARGET lilv::lilv
        APPEND
        PROPERTY INTERFACE_LINK_LIBRARIES sord::sord
      )
    endif()
  endif()
endif()
