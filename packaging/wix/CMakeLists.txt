# Custom action DLL for WiX installer upgrade detection.
# Finds old Mixxx installations with CPACK_WIX_INSTALL_SCOPE "none"
# that the standard FindRelatedProducts action skips due to scope mismatch.
if(WIN32)
  add_library(mixxxupgradecustomaction SHARED mixxxupgradecustomaction.cpp)
  target_link_libraries(mixxxupgradecustomaction PRIVATE msi)
  set_target_properties(
    mixxxupgradecustomaction
    PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/packaging/wix"
      PREFIX ""
  )
endif()
