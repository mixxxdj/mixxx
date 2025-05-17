# Findunofficial-bit7z.cmake - Locate the unofficial bit7z package
#
# Defines:
#   unofficial-bit7z_FOUND     - TRUE if bit7z was found
#   unofficial-bit7z_INCLUDE_DIR - Path to bit7z headers
#   unofficial-bit7z_LIBRARY   - Path to bit7z library
#
# Usage:
#   find_package(unofficial-bit7z CONFIG REQUIRED)
#   target_link_libraries(your_target PRIVATE unofficial::bit7z::bit7z64)

find_path(
  unofficial-bit7z_INCLUDE_DIR
  NAMES bit7z.hpp
  PATHS
    "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include"
    "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include/bit7z"
  PATH_SUFFIXES bit7z
)

message(
  STATUS
  "Searching for bit7z headers in: ${unofficial-bit7z_INCLUDE_DIR}"
)

find_library(
  unofficial-bit7z_LIBRARY
  NAMES bit7z bit7z64
  PATHS
    "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib"
    "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/lib"
)

message(STATUS "Searching for bit7z libraries in: ${unofficial-bit7z_LIBRARY}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  unofficial-bit7z
  REQUIRED_VARS unofficial-bit7z_INCLUDE_DIR unofficial-bit7z_LIBRARY
)

if(unofficial-bit7z_FOUND)
  message(STATUS "Found bit7z includes: ${unofficial-bit7z_INCLUDE_DIR}")
  message(STATUS "Found bit7z library: ${unofficial-bit7z_LIBRARY}")

  if(NOT TARGET unofficial::bit7z::bit7z64)
    add_library(unofficial::bit7z::bit7z64 UNKNOWN IMPORTED)
    set_target_properties(
      unofficial::bit7z::bit7z64
      PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${unofficial-bit7z_INCLUDE_DIR}"
        IMPORTED_LOCATION "${unofficial-bit7z_LIBRARY}"
    )
  endif()
endif()

mark_as_advanced(unofficial-bit7z_INCLUDE_DIR unofficial-bit7z_LIBRARY)
