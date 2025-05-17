#[=======================================================================[.rst:
Findunofficial-bit7z
-------------------

Finds the unofficial bit7z library.

Imported Targets
^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``unofficial::bit7z::bit7z64``
  The bit7z library

Result Variables
^^^^^^^^^^^^^^^

This will define the following variables:

``unofficial-bit7z_FOUND``
  True if the system has the bit7z library.
``unofficial-bit7z_INCLUDE_DIRS``
  Include directories needed to use bit7z.
``unofficial-bit7z_LIBRARIES``
  Libraries needed to link to bit7z.

Cache Variables
^^^^^^^^^^^^^^

The following cache variables may also be set:

``unofficial-bit7z_INCLUDE_DIR``
  The directory containing ``bit7z.hpp``.
``unofficial-bit7z_LIBRARY``
  The path to the bit7z library.

#]=======================================================================]

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_bit7z QUIET unofficial-bit7z)
endif()

find_path(
  unofficial-bit7z_INCLUDE_DIR
  NAMES bit7z.hpp
  PATH_SUFFIXES bit7z
  HINTS
    ${PC_bit7z_INCLUDE_DIRS}
    "$ENV{VCPKG_INSTALLED_DIR}/$ENV{VCPKG_DEFAULT_TRIPLET}/include"
    "/usr/local/include"
    "/usr/include"
  DOC "bit7z include directory"
)
mark_as_advanced(unofficial-bit7z_INCLUDE_DIR)

find_library(
  unofficial-bit7z_LIBRARY
  NAMES bit7z bit7z64
  HINTS
    ${PC_bit7z_LIBRARY_DIRS}
    "$ENV{VCPKG_INSTALLED_DIR}/$ENV{VCPKG_DEFAULT_TRIPLET}/lib"
    "/usr/local/lib"
    "/usr/lib"
  DOC "bit7z library"
)
mark_as_advanced(unofficial-bit7z_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  unofficial-bit7z
  REQUIRED_VARS unofficial-bit7z_LIBRARY unofficial-bit7z_INCLUDE_DIR
  FAIL_MESSAGE "bit7z not found. Install via:\n  vcpkg install bit7z"
)

if(unofficial-bit7z_FOUND)
  set(unofficial-bit7z_LIBRARIES "${unofficial-bit7z_LIBRARY}")
  set(unofficial-bit7z_INCLUDE_DIRS "${unofficial-bit7z_INCLUDE_DIR}")

  if(NOT TARGET unofficial::bit7z::bit7z64)
    add_library(unofficial::bit7z::bit7z64 UNKNOWN IMPORTED)
    set_target_properties(
      unofficial::bit7z::bit7z64
      PROPERTIES
        IMPORTED_LOCATION "${unofficial-bit7z_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${unofficial-bit7z_INCLUDE_DIR}"
    )

    # Handle DLL copy on Windows
    if(WIN32)
      find_file(
        BIT7Z_DLL
        NAMES 7zip.dll
        HINTS
          "$ENV{VCPKG_INSTALLED_DIR}/$ENV{VCPKG_DEFAULT_TRIPLET}/bin"
          "${CMAKE_CURRENT_LIST_DIR}/../../bin"
        NO_DEFAULT_PATH
      )
      if(BIT7Z_DLL AND TARGET mixxx-lib)
        add_custom_command(
          TARGET mixxx-lib
          POST_BUILD
          COMMAND
            ${CMAKE_COMMAND} -E copy_if_different "${BIT7Z_DLL}"
            "$<TARGET_FILE_DIR:mixxx-lib>"
          COMMENT "Copying 7zip.dll to output directory"
        )
      endif()
    endif()
  endif()
endif()
