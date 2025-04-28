#[=======================================================================[.rst:
FindGPerfTools
--------------

Finds the GPerfTools library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``GPerfTools::tcmalloc``
  The GPerfTools tcmalloc library
``GPerfTools::profiler``
  The GPerfTools profiler library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``GPerfTools_FOUND``
  True if the system has the GPerfTools library.
``GPerfTools_INCLUDE_DIRS``
  Include directories needed to use GPerfTools.
``GPerfTools_LIBRARIES``
  Libraries needed to link to GPerfTools.
``GPerfTools_DEFINITIONS``
  Compile definitions needed to use GPerfTools.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``GPerfTools_TCMALLOC_INCLUDE_DIR``
  The directory containing ``gperftools/tcmalloc.h``.
``GPerfTools_TCMALLOC_LIBRARY``
  The path to the GPerfTools tcmalloc library.
``GPerfTools_PROFILER_INCLUDE_DIR``
  The directory containing ``gperftools/profiler.h``.
``GPerfTools_PROFILER_LIBRARY``
  The path to the GPerfTools profiler library.

#]=======================================================================]

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_GPerfTools_TCMALLOC QUIET libtcmalloc)
  pkg_check_modules(PC_GPerfTools_PROFILER QUIET libprofiler)
endif()

find_path(
  GPerfTools_TCMALLOC_INCLUDE_DIR
  NAMES gperftools/tcmalloc.h
  HINTS ${PC_GPerfTools_TCMALLOC_INCLUDE_DIRS}
  DOC "tcmalloc include directory"
)
mark_as_advanced(GPerfTools_TCMALLOC_INCLUDE_DIR)

find_library(
  GPerfTools_TCMALLOC_LIBRARY
  NAMES tcmalloc
  HINTS ${PC_GPerfTools_TCMALLOC_LIBRARY_DIRS}
  DOC "tcmalloc library"
)
mark_as_advanced(GPerfTools_TCMALLOC_LIBRARY)

find_path(
  GPerfTools_PROFILER_INCLUDE_DIR
  NAMES gperftools/profiler.h
  HINTS ${PC_GPerfTools_PROFILER_INCLUDE_DIRS}
  DOC "profiler include directory"
)
mark_as_advanced(GPerfTools_PROFILER_INCLUDE_DIR)

find_library(
  GPerfTools_PROFILER_LIBRARY
  NAMES profiler
  HINTS ${PC_GPerfTools_PROFILER_LIBRARY_DIRS}
  DOC "profiler library"
)
mark_as_advanced(GPerfTools_PROFILER_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  GPerfTools
  DEFAULT_MSG
  GPerfTools_TCMALLOC_LIBRARY
  GPerfTools_TCMALLOC_INCLUDE_DIR
  GPerfTools_PROFILER_LIBRARY
  GPerfTools_PROFILER_INCLUDE_DIR
)

if(GPerfTools_FOUND)
  set(
    GPerfTools_LIBRARIES
    ${GPerfTools_TCMALLOC_LIBRARY}
    ${GPerfTools_PROFILER_LIBRARY}
  )
  set(
    GPerfTools_INCLUDE_DIRS
    ${GPerfTools_TCMALLOC_INCLUDE_DIR}
    ${GPerfTools_PROFILER_INCLUDE_DIR}
  )
  set(
    GPerfTools_DEFINITIONS
    ${PC_GPerfTools_TCMALLOC_CFLAGS_OTHER}
    ${PC_GPerfTools_PROFILER_CFLAGS_OTHER}
  )

  if(NOT TARGET GPerfTools::tcmalloc)
    add_library(GPerfTools::tcmalloc UNKNOWN IMPORTED)
    set_target_properties(
      GPerfTools::tcmalloc
      PROPERTIES
        IMPORTED_LOCATION ${GPerfTools_TCMALLOC_LIBRARY}
        INTERFACE_COMPILE_OPTIONS "${PC_GPerfTools_TCMALLOC_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${GPerfTools_TCMALLOC_INCLUDE_DIR}"
    )
  endif()
  if(NOT TARGET GPerfTools::profiler)
    add_library(GPerfTools::profiler UNKNOWN IMPORTED)
    set_target_properties(
      GPerfTools::profiler
      PROPERTIES
        IMPORTED_LOCATION "${GPerfTools_PROFILER_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_GPerfTools_PROFILER_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${GPerfTools_PROFILER_INCLUDE_DIR}"
    )
  endif()
endif()
