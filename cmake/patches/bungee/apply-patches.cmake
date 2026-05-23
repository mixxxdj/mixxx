# cmake/patches/bungee/apply-patches.cmake
#
# Applies the Bungee source-tree patches required by Mixxx's
# ExternalProject_Add fallback.  Invoked by CMake from
# ExternalProject_Add's PATCH_COMMAND once the upstream tarball has been
# extracted; the working directory is the extracted source root.
#
# Required -D variables (passed by the parent build):
#   MIXXX_BUNGEE_PATCH_DIR        — directory holding the permanent Bungee
#                                   patch set (cmake/patches/bungee/).
#   MIXXX_BUNGEE_PATCH_EXECUTABLE — path to GNU `patch`.
#   MIXXX_BUNGEE_APPLY_MSVC_PATCHES — TRUE/FALSE: whether to apply the two
#                                   MSVC-only Bungee patches (only on MSVC
#                                   builds; ignored elsewhere).
#
# All patches are applied with `patch -p1 -l -N`:
#   -p1  strip one leading path component
#   -l   --ignore-whitespace  (the upstream Bungee tarball ships trailing
#         whitespace on lines that the upstream-derived patches normalise away)
#   -N   --forward (don't try reverse-applied detection — fail fast instead)

if(
  NOT DEFINED MIXXX_BUNGEE_PATCH_EXECUTABLE
  OR MIXXX_BUNGEE_PATCH_EXECUTABLE STREQUAL ""
)
  message(
    FATAL_ERROR
    "apply-patches.cmake: MIXXX_BUNGEE_PATCH_EXECUTABLE is required"
  )
endif()
if(NOT DEFINED MIXXX_BUNGEE_PATCH_DIR OR MIXXX_BUNGEE_PATCH_DIR STREQUAL "")
  message(FATAL_ERROR "apply-patches.cmake: MIXXX_BUNGEE_PATCH_DIR is required")
endif()

# Patch ordering keeps every provider on the same dependency-layout fixes
# before applying optional MSVC compatibility patches. Apply the CMake-minimum
# patch last so it anchors on the dependency-layout-adjusted tree.
set(
  _patches
  "${MIXXX_BUNGEE_PATCH_DIR}/cmake-use-vcpkg-deps-and-install-layout.patch"
  "${MIXXX_BUNGEE_PATCH_DIR}/pffft-include-path.patch"
)
if(MIXXX_BUNGEE_APPLY_MSVC_PATCHES)
  list(
    APPEND
    _patches
    "${MIXXX_BUNGEE_PATCH_DIR}/assert-win32-compat.patch"
    "${MIXXX_BUNGEE_PATCH_DIR}/resample-msvc-noinline.patch"
  )
endif()
list(APPEND _patches "${MIXXX_BUNGEE_PATCH_DIR}/lower-cmake-minimum.patch")

foreach(_patch IN LISTS _patches)
  if(NOT EXISTS "${_patch}")
    message(FATAL_ERROR "Bungee patch not found: ${_patch}")
  endif()
  message(STATUS "Applying Bungee patch: ${_patch}")
  execute_process(
    COMMAND "${MIXXX_BUNGEE_PATCH_EXECUTABLE}" -p1 -l -N -i "${_patch}"
    RESULT_VARIABLE _rc
    OUTPUT_VARIABLE _out
    ERROR_VARIABLE _err
  )
  if(NOT _rc EQUAL 0)
    message(
      FATAL_ERROR
      "Failed to apply Bungee patch: ${_patch}\n"
      "exit=${_rc}\nstdout:\n${_out}\nstderr:\n${_err}"
    )
  endif()
endforeach()
