# unlike CMakeLists.txt this file is include at cpack time, once per generator after CPack has set CPACK_GENERATOR
# to the actual generator being used. It allows per-generator setting of CPACK_* variables at cpack time.

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
