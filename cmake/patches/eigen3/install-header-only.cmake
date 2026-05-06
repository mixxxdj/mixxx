# Install a minimal header-only Eigen3 package for Mixxx's Bungee
# ExternalProject_Add source-fetch fallback.
#
# Eigen is header-only for Bungee's use case.  Avoid running Eigen's own CMake
# configure step here: it always descends into the optional blas/lapack
# subdirectories, which enables Fortran even when tests/builds are disabled and
# breaks MSVC/cross-toolchain CI environments that do not have a matching
# Fortran compiler.

if(NOT DEFINED SOURCE_DIR OR SOURCE_DIR STREQUAL "")
  message(FATAL_ERROR "SOURCE_DIR is required")
endif()

if(NOT DEFINED INSTALL_DIR OR INSTALL_DIR STREQUAL "")
  message(FATAL_ERROR "INSTALL_DIR is required")
endif()

set(_include_dir "${INSTALL_DIR}/include/eigen3")
set(_config_dir "${INSTALL_DIR}/share/eigen3/cmake")

file(MAKE_DIRECTORY "${_include_dir}" "${_config_dir}")
file(COPY "${SOURCE_DIR}/Eigen" DESTINATION "${_include_dir}")

if(EXISTS "${SOURCE_DIR}/unsupported")
  file(COPY "${SOURCE_DIR}/unsupported" DESTINATION "${_include_dir}")
endif()

file(
  WRITE
  "${_config_dir}/Eigen3Config.cmake"
  [=[
get_filename_component(_eigen3_prefix "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)

if(NOT TARGET Eigen3::Eigen)
  add_library(Eigen3::Eigen INTERFACE IMPORTED)
  set_target_properties(
    Eigen3::Eigen
    PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${_eigen3_prefix}/include/eigen3"
  )
endif()

set(Eigen3_FOUND TRUE)
set(EIGEN3_FOUND TRUE)
set(Eigen3_VERSION "3.4.0")
set(EIGEN3_VERSION_STRING "3.4.0")
set(EIGEN3_INCLUDE_DIR "${_eigen3_prefix}/include/eigen3")
]=]
)
