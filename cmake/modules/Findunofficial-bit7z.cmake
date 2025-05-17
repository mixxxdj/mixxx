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

# Findunofficial-bit7z.cmake
# Locates the bit7z library installed via vcpkg

set(
  VCPKG_POSSIBLE_INCLUDE_PATHS
  "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include"
  "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include/bit7z"
  "$ENV{VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/include"
  "$ENV{VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/include/bit7z"
)

# Find headers
find_path(
  BIT7Z_INCLUDE_DIR
  bit7z.hpp
  PATHS ${VCPKG_POSSIBLE_INCLUDE_PATHS}
  PATH_SUFFIXES bit7z
  DOC "Path to bit7z headers"
)

# Library search paths
set(
  VCPKG_POSSIBLE_LIBRARY_PATHS
  "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib"
  "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/lib"
)

# Find library
find_library(
  BIT7Z_LIBRARY
  NAMES bit7z bit7z64
  PATHS ${VCPKG_POSSIBLE_LIBRARY_PATHS}
  DOC "Path to bit7z library"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  unofficial-bit7z
  REQUIRED_VARS BIT7Z_INCLUDE_DIR BIT7Z_LIBRARY
)

if(unofficial-bit7z_FOUND AND NOT TARGET unofficial::bit7z::bit7z64)
  add_library(unofficial::bit7z::bit7z64 UNKNOWN IMPORTED)
  set_target_properties(
    unofficial::bit7z::bit7z64
    PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${BIT7Z_INCLUDE_DIR}"
      IMPORTED_LOCATION "${BIT7Z_LIBRARY}"
  )
endif()

# Windows DLL handling (now properly attached to your main target)
if(WIN32 AND unofficial-bit7z_FOUND)
  find_file(
    BIT7Z_DLL
    NAMES 7zip.dll
    PATHS
      "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin"
      "$ENV{VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/bin"
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

mark_as_advanced(BIT7Z_INCLUDE_DIR BIT7Z_LIBRARY BIT7Z_DLL)
