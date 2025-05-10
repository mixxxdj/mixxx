# Find7zip.cmake - Locate the 7-Zip SDK
#
# Defines:
#   7zip_FOUND          - TRUE if 7zip was found
#   7zip_INCLUDE_DIR    - Path to 7zip headers
#   7zip_LIBRARY        - Path to 7zip static or shared library
#
# Usage:
#   find_package(7zip REQUIRED)
#   target_include_directories(your_target PRIVATE ${7zip_INCLUDE_DIR})
#   target_link_libraries(your_target PRIVATE ${7zip_LIBRARY})

find_path(
  7zip_INCLUDE_DIR
  # NAMES 7z.h
  NAMES 7z.h 7zpp.h
  PATHS
    # B:/vcpkg-git/vcpkg/vcpkg_installed/x64-windows/include/7zip/C
    "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include"
    "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include/7zip"
    "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include/7zpp"
  PATH_SUFFIXES 7zip 7zpp C
)

# Output the path being searched for headers
message(STATUS "Searching for 7zip headers in: ${7zip_INCLUDE_DIR}")

# Find 7zip library
find_library(
  7zip_LIBRARY
  # NAMES 7zip
  NAMES 7z 7zip 7zpp 7zxa 7zra 7zr 7za
  PATHS
    # B:/vcpkg-git/vcpkg/vcpkg_installed/x64-windows/lib
    # B:/vcpkg-git/vcpkg/vcpkg_installed/x64-windows/debug/lib
    "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib"
    "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/lib"
)

# Output the path being searched for libraries
message(STATUS "Searching for 7zip libraries in: ${7zip_LIBRARY}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  7zip
  REQUIRED_VARS 7zip_INCLUDE_DIR 7zip_LIBRARY
)

if(7zip_FOUND)
  message(STATUS "Found 7zip includes: ${7zip_INCLUDE_DIR}")
  message(STATUS "Found 7zip library: ${7zip_LIBRARY}")
endif()

mark_as_advanced(7zip_INCLUDE_DIR 7zip_LIBRARY)
