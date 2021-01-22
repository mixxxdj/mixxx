# CPACK_SOURCE_DIR is set to CMAKE_CURRENT_SOURCE_DIR in the root CMakeLists.txt
# because CPack sets CMAKE_CURRENT_SOURCE_DIR to the path it is about to copy
# the source into, but it has not yet copied the source when this script is run.
set(GIT_REPO_ROOT "${CPACK_SOURCE_DIR}")
include("${CPACK_SOURCE_DIR}/cmake/GitVersionInfo.cmake")

configure_file(
  "${CPACK_SOURCE_DIR}/packaging/VersionInfo.cmake.in"
  "${CPACK_SOURCE_DIR}/packaging/VersionInfo.cmake"
  @ONLY
)
