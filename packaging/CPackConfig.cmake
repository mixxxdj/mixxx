# unlike CMakeLists.txt this file is include at cpack time, once per generator after CPack has set CPACK_GENERATOR
# to the actual generator being used. It allows per-generator setting of CPACK_* variables at cpack time.
set(GIT_DESCRIBE "${CPACK_GIT_DESCRIBE}")
set(GIT_COMMIT_DATE ${CPACK_GIT_COMMIT_DATE})
set(CMAKE_CURRENT_SOURCE_DIR "${CPACK_SOURCE_DIR}")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
include(GitInfo)

if(NOT GIT_DESCRIBE)
  set(PACKAGE_VERSION "${CPACK_MIXXX_VERSION}-unknown")
else()
  set(PACKAGE_VERSION "${GIT_DESCRIBE}")
endif()
set(CPACK_PACKAGE_FILE_NAME "mixxx-${PACKAGE_VERSION}")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}-source")

# The upstream version must not contain hyphen
# . for nomal versioning + for advance and ~ for decline the version
# dpkg --compare-versions 2.3~alpha~1234~g8163 lt 2.3~beta~1234~g8163 && echo true
# dpkg --compare-versions 2.3~beta~1234~g8163 lt 2.3.0 && echo true
# dpkg --compare-versions 2.3.0 lt 2.3.0+2345+g163  && echo true
if (PACKAGE_VERSION MATCHES "^[0-9][A-Za-z0-9.+~-]*$")
  if (PACKAGE_VERSION MATCHES "(alpha|beta)")
    string(REPLACE "-" "~" CPACK_DEBIAN_PACKAGE_VERSION "${PACKAGE_VERSION}")
  else()
    string(REPLACE "-" "+" CPACK_DEBIAN_PACKAGE_VERSION "${PACKAGE_VERSION}")
  endif()
else()
  string(REPLACE "-" "~" CPACK_DEBIAN_PACKAGE_VERSION "${CPACK_MIXXX_VERSION}")
endif()

if (CPACK_GENERATOR STREQUAL "DEB")
  set(CPACK_INSTALL_SCRIPT ${CPACK_DEBIAN_INSTALL_SCRIPT})
endif()

if (CPACK_GENERATOR STREQUAL "External")
  if (DEB_SOURCEPKG OR DEB_UPLOAD_PPA OR DEB_BUILD)
    set(CPACK_EXTERNAL_ENABLE_STAGING true)
    set(CPACK_INSTALLED_DIRECTORIES "${CPACK_DEBIAN_SOURCE_DIR};/")
    set(CPACK_IGNORE_FILES "${CPACK_SOURCE_IGNORE_FILES}")
    set(CPACK_INSTALL_CMAKE_PROJECTS "")
    set(CPACK_EXTERNAL_PACKAGE_SCRIPT "${CPACK_DEBIAN_UPLOAD_PPA_SCRIPT}" )
  endif ()
endif()
