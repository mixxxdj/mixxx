# unlike CMakeLists.txt this file is include at cpack time, once per generator after CPack has set CPACK_GENERATOR
# to the actual generator being used. It allows per-generator setting of CPACK_* variables at cpack time.

if (CPACK_GENERATOR MATCHES "DEB")
  set( CPACK_INSTALL_SCRIPT ${CPACK_DEBIAN_INSTALL_SCRIPT} )
endif()
