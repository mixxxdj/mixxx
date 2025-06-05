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

# Find 7zip includes and libraries

# Find the include directory where 7zVersion.h or 7zip/C/7z.h are located
find_path(
  7zip_INCLUDE_DIR
  NAMES 7zVersion.h 7zip/C/7z.h
  PATH_SUFFIXES 7zip/C 7zip/CPP include/7zip include
  PATHS /usr/include /usr/local/include /usr/lib /usr/local/lib
  DOC "7zip include directory"
)
mark_as_advanced(7zip_INCLUDE_DIR)

# Find the 7zip library (it could be 7z.dll, lib7z.so, or lib7z.dylib)
find_library(
  7zip_LIBRARY
  NAMES 7zip 7z lib7z
  PATH_SUFFIXES bin lib
  DOC "7zip library"
)

mark_as_advanced(7zip_LIBRARY)

# Ensure versioning if provided by the user
if(DEFINED 7zip_VERSION AND NOT 7zip_VERSION STREQUAL "")
  set(7zip_VERSION "${7zip_VERSION}")
endif()

# Handle standard results (if found, set results)
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  7zip
  REQUIRED_VARS 7zip_LIBRARY 7zip_INCLUDE_DIR
  VERSION_VAR 7zip_VERSION
)

if(7zip_FOUND)
  # Create the imported target for 7zip if not already created
  if(NOT TARGET 7zip::7zip)
    add_library(7zip::7zip UNKNOWN IMPORTED)

    # Set the properties for the imported target
    set_target_properties(
      7zip::7zip
      PROPERTIES
        IMPORTED_LOCATION "${7zip_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${7zip_INCLUDE_DIR}"
    )

    # Optionally, add any compile flags if needed (e.g., CFLAGS_OTHER)
    if(DEFINED 7zip_CFLAGS_OTHER)
      target_compile_options(7zip::7zip INTERFACE ${7zip_CFLAGS_OTHER})
    endif()
  endif()
endif()
