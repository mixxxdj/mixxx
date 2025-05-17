# Findunofficial-bit7z.cmake
# Enhanced version with multi-platform support and better diagnostics

# Set debug output
message(STATUS "Searching for unofficial-bit7z package...")

# 1. Define search paths --------------------------------------------------------
set(SEARCH_PATHS)
list(
  APPEND
  SEARCH_PATHS
  # VCPKG standard locations
  "$ENV{VCPKG_ROOT}/installed/$ENV{VCPKG_DEFAULT_TRIPLET}"
  "$ENV{VCPKG_INSTALLED_DIR}/$ENV{VCPKG_DEFAULT_TRIPLET}"
  # Common system paths
  "/usr/local"
  "/usr"
  "/opt/local"
)

# 2. Find headers --------------------------------------------------------------
find_path(
  BIT7Z_INCLUDE_DIR
  NAMES bit7z.hpp
  PATHS ${SEARCH_PATHS}
  PATH_SUFFIXES include include/bit7z bit7z/include
  NO_DEFAULT_PATH
)

# 3. Find library --------------------------------------------------------------
set(BIT7Z_LIB_NAMES bit7z bit7z64)

find_library(
  BIT7Z_LIBRARY
  NAMES ${BIT7Z_LIB_NAMES}
  PATHS ${SEARCH_PATHS}
  PATH_SUFFIXES lib lib64 bin Debug/lib Release/lib
  NO_DEFAULT_PATH
)

# 4. Debug output --------------------------------------------------------------
message(STATUS "BIT7Z_INCLUDE_DIR search locations: ${SEARCH_PATHS}")
message(STATUS "Found BIT7Z_INCLUDE_DIR: ${BIT7Z_INCLUDE_DIR}")
message(STATUS "Found BIT7Z_LIBRARY: ${BIT7Z_LIBRARY}")

# 5. Final verification --------------------------------------------------------
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  unofficial-bit7z
  REQUIRED_VARS BIT7Z_INCLUDE_DIR BIT7Z_LIBRARY
  FAIL_MESSAGE "bit7z not found. Install via:\n  vcpkg install bit7z"
)

# 6. Create target if found ----------------------------------------------------
if(unofficial-bit7z_FOUND AND NOT TARGET unofficial::bit7z::bit7z64)
  message(STATUS "Creating imported target unofficial::bit7z::bit7z64")

  add_library(unofficial::bit7z::bit7z64 UNKNOWN IMPORTED)
  set_target_properties(
    unofficial::bit7z::bit7z64
    PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${BIT7Z_INCLUDE_DIR}"
      IMPORTED_LOCATION "${BIT7Z_LIBRARY}"
  )
endif()
