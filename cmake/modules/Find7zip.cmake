#[=======================================================================[.rst:
Find7zip
--------

Finds the 7-Zip library (7z.dll or lib7z.so and import library).

Imported Targets
^^^^^^^^^^^^^^^^
``7zip::7zip``

Result Variables
^^^^^^^^^^^^^^^^
``7zip_FOUND``, ``7zip_INCLUDE_DIRS``, ``7zip_LIBRARIES``

#]=======================================================================]

include(IsStaticLibrary)

find_path(
  7zip_INCLUDE_DIR
  NAMES 7zVersion.h
  PATH_SUFFIXES 7zip/C 7zip 7-Zip CPP C
  DOC "7-Zip include directory"
)

find_library(7zip_LIBRARY NAMES 7z 7zip DOC "7-Zip library")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  7zip
  REQUIRED_VARS 7zip_LIBRARY 7zip_INCLUDE_DIR
)

if(7zip_FOUND)
  set(7zip_LIBRARIES "${7zip_LIBRARY}")
  set(7zip_INCLUDE_DIRS "${7zip_INCLUDE_DIR}")

  if(NOT TARGET 7zip::7zip)
    add_library(7zip::7zip UNKNOWN IMPORTED)
    set_target_properties(
      7zip::7zip
      PROPERTIES
        IMPORTED_LOCATION "${7zip_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${7zip_INCLUDE_DIR}"
    )
    is_static_library(7zip_IS_STATIC 7zip::7zip)
    if(7zip_IS_STATIC AND WIN32)
      # Optional: Define anything specific for static linking on Windows.
    endif()
  endif()
endif()
