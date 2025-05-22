#[=======================================================================[.rst:
Find7zip
--------

Finds the 7-Zip library (7z.dll/lib7z.so/lib7z.dylib and headers).

Imported Targets
^^^^^^^^^^^^^^^^
``7zip::7zip``

Result Variables
^^^^^^^^^^^^^^^^
``7zip_FOUND``, ``7zip_INCLUDE_DIRS``, ``7zip_LIBRARIES``
#]=======================================================================]

# find_package(PkgConfig QUIET)
# if(PkgConfig_FOUND)
#   pkg_check_modules(7zip QUIET 7zip)
# endif()

find_path(
  7zip_INCLUDE_DIR
  # NAMES 7zip/C/7zip.h
  NAMES 7zVersion.h 7zip/C/7z.h
  PATH_SUFFIXES 7zip/C 7zip/CPP include/7zip include
  PATHS /usr/include /usr/local/include
  # HINTS ${7zip_INCLUDE_DIRS}
  DOC "7zip include directory"
)
mark_as_advanced(7zip_INCLUDE_DIR)

find_library(
  7zip_LIBRARY
  NAMES 7zip 7z lib7z
  PATH_SUFFIXES bin lib
  #HINTS ${7zip_LIBRARY_DIRS}
  DOC "7zip library"
)
mark_as_advanced(7zip_LIBRARY)

if(DEFINED 7zip_VERSION AND NOT 7zip_VERSION STREQUAL "")
  set(7zip_VERSION "${7zip_VERSION}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  7zip
  REQUIRED_VARS 7zip_LIBRARY 7zip_INCLUDE_DIR
  VERSION_VAR 7zip_VERSION
)

if(7zip_FOUND)
  if(NOT TARGET 7zip::7zip)
    add_library(7zip::7zip UNKNOWN IMPORTED)
    set_target_properties(
      7zip::7zip
      PROPERTIES
        IMPORTED_LOCATION "${7zip_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${7zip_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${7zip_INCLUDE_DIR}"
    )
  endif()
endif()
