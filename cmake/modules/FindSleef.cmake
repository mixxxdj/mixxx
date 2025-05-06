#[=======================================================================[.rst:
FindSleef
--------------

Finds the Sleef library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``Sleef::sleef``
  The Sleef library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``Sleef_FOUND``
  True if the system has the Sleef library.
``Sleef_INCLUDE_DIRS``
  Include directories needed to use Sleef.
``Sleef_LIBRARIES``
  Libraries needed to link to Sleef.
``Sleef_DEFINITIONS``
  Compile definitions needed to use Sleef.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``Sleef_INCLUDE_DIR``
  The directory containing ``sleef.h``.
``Sleef_LIBRARY``
  The path to the Sleef library.

#]=======================================================================]

include(IsStaticLibrary)

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_Sleef QUIET sleef)
endif()

find_path(
  Sleef_INCLUDE_DIR
  NAMES sleef.h
  PATHS ${PC_sleef_INCLUDE_DIRS}
  DOC "Sleef include directory"
)
mark_as_advanced(Sleef_INCLUDE_DIR)

find_library(
  Sleef_LIBRARY
  NAMES ${PC_Sleef_LIBRARIES}
  PATHS ${PC_Sleef_LIBRARY_DIRS}
)
mark_as_advanced(Sleef_LIBRARY)

find_library(SleefDFT_LIBRARY NAMES sleefdft PATHS ${PC_Sleef_LIBRARY_DIRS})
mark_as_advanced(SleefDFT_LIBRARY)

if(DEFINED PC_Sleef_VERSION AND NOT PC_Sleef_VERSION STREQUAL "")
  set(Sleef_VERSION "${PC_Sleef_VERSION}")
else()
  if(EXISTS "${sleef_INCLUDE_DIR}/sleef.h")
    file(READ "$sleef{SLEEF_INCLUDE_DIR}/sleef.h" SLEEF_FIND_HEADER_CONTENTS)

    set(SLEEF_MAJOR_PREFIX "#define SLEEF_VERSION_MAJOR ")
    set(SLEEF_MINOR_PREFIX "#define SLEEF_VERSION_MINOR ")
    set(SLEEF_PATCH_PREFIX "#define SLEEF_VERSION_PATCHLEVEL ")

    string(
      REGEX MATCH
      "${SLEEF_MAJOR_PREFIX}[0-9]+"
      SLEEF_MAJOR_VERSION
      "${SLEEF_FIND_HEADER_CONTENTS}"
    )
    string(
      REPLACE
      "${SLEEF_MAJOR_PREFIX}"
      ""
      SLEEF_MAJOR_VERSION
      "${SLEEF_MAJOR_VERSION}"
    )

    string(
      REGEX MATCH
      "${SLEEF_MINOR_PREFIX}[0-9]+"
      SLEEF_MINOR_VERSION
      "${SLEEF_FIND_HEADER_CONTENTS}"
    )
    string(
      REPLACE
      "${SLEEF_MINOR_PREFIX}"
      ""
      SLEEF_MINOR_VERSION
      "${SLEEF_MINOR_VERSION}"
    )

    string(
      REGEX MATCH
      "${SLEEF_PATCH_PREFIX}[0-9]+"
      SLEEF_SUBMINOR_VERSION
      "${SLEEF_FIND_HEADER_CONTENTS}"
    )
    string(
      REPLACE
      "${SLEEF_PATCH_PREFIX}"
      ""
      SLEEF_SUBMINOR_VERSION
      "${SLEEF_SUBMINOR_VERSION}"
    )

    set(
      Sleef_VERSION
      "${SLEEF_MAJOR_VERSION}.${SLEEF_MINOR_VERSION}.${SLEEF_SUBMINOR_VERSION}"
    )
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Sleef
  REQUIRED_VARS Sleef_INCLUDE_DIR Sleef_LIBRARY
  VERSION_VAR Sleef_VERSION
)

if(Sleef_FOUND)
  set(Sleef_LIBRARIES "${Sleef_LIBRARY}")
  set(Sleef_INCLUDE_DIRS "${Sleef_INCLUDE_DIR}")
  set(Sleef_DEFINITIONS ${PC_Sleef_CFLAGS_OTHER})

  if(SleefDFT_LIBRARY AND NOT TARGET Sleef::sleefdft)
    add_library(Sleef::sleefdft UNKNOWN IMPORTED)
    set_target_properties(
      Sleef::sleefdft
      PROPERTIES
        IMPORTED_LOCATION "${SleefDFT_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_Sleef_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${Sleef_INCLUDE_DIR}"
    )

    is_static_library(SleefDFT_IS_STATIC Sleef::sleefdft)
    if(SleefDFT_IS_STATIC)
      set_property(
        TARGET Sleef::sleefdft
        APPEND
        PROPERTY INTERFACE_LINK_LIBRARIES Sleef::sleef
      )

      find_package(OpenMP)
      if(OpenMP_CXX_FOUND)
        set_property(
          TARGET Sleef::sleefdft
          APPEND
          PROPERTY INTERFACE_LINK_LIBRARIES OpenMP::OpenMP_CXX
        )
      endif()
    endif()
  endif()

  if(NOT TARGET Sleef::sleef)
    add_library(Sleef::sleef UNKNOWN IMPORTED)
    set_target_properties(
      Sleef::sleef
      PROPERTIES
        IMPORTED_LOCATION "${Sleef_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_Sleef_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${Sleef_INCLUDE_DIR}"
    )
  endif()
endif()
