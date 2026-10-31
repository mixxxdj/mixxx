#[=======================================================================[.rst:
FindSQLite3
-----------

Finds the SQLite3 library.

vcpkg uses an ``unofficial-`` prefix for ports where upstream does not ship
CMake config files. This module tries vcpkg's ``unofficial-sqlite3`` first,
then falls back to CMake's built-in ``FindSQLite3`` module.

Imported Targets
^^^^^^^^^^^^^^^^

``SQLite::SQLite3``
  The SQLite3 library

Result Variables
^^^^^^^^^^^^^^^^

``SQLite3_FOUND``
  True if the system has the SQLite3 library.
``SQLite3_INCLUDE_DIRS``
  Include directories needed to use SQLite3.
``SQLite3_VERSION``
  The version of SQLite3 found.

#]=======================================================================]

# Avoid repeated processing
if(TARGET SQLite::SQLite3)
  set(SQLite3_FOUND TRUE)
  return()
endif()

# Try vcpkg's unofficial-sqlite3 package first (CONFIG mode only)
# vcpkg prefixes packages with "unofficial-" when upstream doesn't provide
# CMake config files. These packages expose only a target for linking;
# so we extract the version from sqlite3.h below.
find_package(unofficial-sqlite3 CONFIG QUIET)

if(TARGET unofficial::sqlite3::sqlite3)
  # Extract the include directory from the imported target
  get_target_property(
    SQLite3_INCLUDE_DIR
    unofficial::sqlite3::sqlite3
    INTERFACE_INCLUDE_DIRECTORIES
  )
  if(SQLite3_INCLUDE_DIR)
    # INTERFACE_INCLUDE_DIRECTORIES may be a list; use the first entry
    list(GET SQLite3_INCLUDE_DIR 0 SQLite3_INCLUDE_DIR)
  endif()

  # Detect version from sqlite3.h
  if(SQLite3_INCLUDE_DIR AND EXISTS "${SQLite3_INCLUDE_DIR}/sqlite3.h")
    file(
      STRINGS
      "${SQLite3_INCLUDE_DIR}/sqlite3.h"
      _sqlite3_version_str
      REGEX "SQLITE_VERSION[\t ]+\""
      LIMIT_COUNT 1
    )
    string(REGEX MATCH "\"([^\"]+)\"" _unused "${_sqlite3_version_str}")
    set(SQLite3_VERSION "${CMAKE_MATCH_1}")
    unset(_sqlite3_version_str)
  endif()

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(
    SQLite3
    REQUIRED_VARS SQLite3_INCLUDE_DIR
    VERSION_VAR SQLite3_VERSION
  )

  if(SQLite3_FOUND AND NOT TARGET SQLite::SQLite3)
    # Use an IMPORTED INTERFACE target instead of ALIAS so that vcpkg's
    # vcpkg-cmake-wrapper.cmake can call set_target_properties() on it.
    # ALIAS targets do not support set_target_properties(), which causes
    # the Android (and potentially other) vcpkg builds to fail.
    add_library(SQLite::SQLite3 INTERFACE IMPORTED GLOBAL)
    target_link_libraries(
      SQLite::SQLite3
      INTERFACE unofficial::sqlite3::sqlite3
    )
    set(SQLite3_INCLUDE_DIRS "${SQLite3_INCLUDE_DIR}")
  endif()

  if(SQLite3_FOUND AND TARGET unofficial::sqlite3::sqlite3)
    # Ensure that the actual library is linked, not just INTERFACE
    get_target_property(
      _sqlite3_libs
      unofficial::sqlite3::sqlite3
      INTERFACE_LINK_LIBRARIES
    )
    # _sqlite3_libs should contain the actual library, but on Android (static linked build) it may not.
    # Force linking the real library if needed.
    if(_sqlite3_libs)
      target_link_libraries(SQLite::SQLite3 INTERFACE ${_sqlite3_libs})
    endif()
  endif()
else()
  # If VCPKG's unofficial-sqlite3 package is not found, fall back to CMake's built-in FindSQLite3 module
  include("${CMAKE_ROOT}/Modules/FindSQLite3.cmake")
endif()
