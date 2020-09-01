# This file is part of Mixxx, Digital DJ'ing software.
# Copyright (C) 2001-2020 Mixxx Development Team
# Distributed under the GNU General Public Licence (GPL) version 2 or any later
# later version. See the LICENSE file for details.

#[=======================================================================[.rst:
FindSQLite3
-----------

Finds the SQLite3 library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``SQLite3::SQLite3``
  The SQLite3 library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``SQLite3_FOUND``
  True if the system has the SQLite3 library.
``SQLite3_INCLUDE_DIRS``
  Include directories needed to use SQLite3.
``SQLite3_LIBRARIES``
  Libraries needed to link to SQLite3.
``SQLite3_DEFINITIONS``
  Compile definitions needed to use SQLite3.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``SQLite3_INCLUDE_DIR``
  The directory containing ``sqlite3.h``.
``SQLite3_LIBRARY``
  The path to the SQLite3 library.

#]=======================================================================]

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_SQLite3 QUIET sqlite3)
endif()

find_path(SQLite3_INCLUDE_DIR
  NAMES sqlite3.h
  PATHS ${PC_SQLite3_INCLUDE_DIRS}
  PATH_SUFFIXES sqlite sqlite3
  DOC "SQLite3 include directory")
mark_as_advanced(SQLite3_INCLUDE_DIR)

find_library(SQLite3_LIBRARY
  NAMES sqlite3
  PATHS ${PC_SQLite3_LIBRARY_DIRS}
  DOC "SQLite3 library"
)
mark_as_advanced(SQLite3_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  SQLite3
  DEFAULT_MSG
  SQLite3_LIBRARY
  SQLite3_INCLUDE_DIR
)

if(SQLite3_FOUND)
  set(SQLite3_LIBRARIES "${SQLite3_LIBRARY}")
  set(SQLite3_INCLUDE_DIRS "${SQLite3_INCLUDE_DIR}")
  set(SQLite3_DEFINITIONS ${PC_SQLite3_CFLAGS_OTHER})

  if(NOT TARGET SQLite3::SQLite3)
    add_library(SQLite3::SQLite3 UNKNOWN IMPORTED)
    set_target_properties(SQLite3::SQLite3
      PROPERTIES
        IMPORTED_LOCATION "${SQLite3_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_SQLite3_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${SQLite3_INCLUDE_DIR}"
    )
  endif()
endif()
