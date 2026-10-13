# AppImage-specific install rules, included from the main CMakeLists.txt
# when MIXXX_APPIMAGE is enabled.

# AppImage desktop file (simplified Exec=mixxx).
# The Exec= key must refer to the binary by basename only, as CPack matches it
# against an installed file.
set(
  MIXXX_APPIMAGE_DESKTOP_FILE
  "${CMAKE_CURRENT_BINARY_DIR}/mixxx-appimage.desktop"
)
file(
  READ
  "${CMAKE_CURRENT_SOURCE_DIR}/res/linux/org.mixxx.Mixxx.desktop"
  MIXXX_APPIMAGE_DESKTOP_CONTENT
)
string(
  REGEX REPLACE
  "\nExec=[^\n]*"
  "\nExec=mixxx"
  MIXXX_APPIMAGE_DESKTOP_CONTENT
  "${MIXXX_APPIMAGE_DESKTOP_CONTENT}"
)
file(
  WRITE
  "${MIXXX_APPIMAGE_DESKTOP_FILE}"
  "${MIXXX_APPIMAGE_DESKTOP_CONTENT}"
)
install(
  FILES "${MIXXX_APPIMAGE_DESKTOP_FILE}"
  DESTINATION "${CMAKE_INSTALL_DATADIR}/applications"
  RENAME "mixxx-appimage.desktop"
)

# Bundle the shared libraries from the VCPKG buildenv that Mixxx links
# against (the binary's RUNPATH is $ORIGIN/../lib, set by CPack, so they
# must end up in AppDir/lib).  The system libraries Mixxx depends on are
# expected to be provided by the host system.
# If a future buildenv build turns another dependency into a shared
# library, add it to the exclusion list below.
file(
  GLOB
  _appimage_buildenv_shared_libraries
  "${MIXXX_VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/lib/*.so*"
)
# System/compiler libraries that must not be bundled into the AppImage.
set(_appimage_system_libraries
    libc.so
    libm.so
    libstdc++.so
    libgcc_s.so
    ld-linux
    libX11.so
    libXext.so
    libXrandr.so
    libxcb.so
    libEGL.so
    libGLX.so
    libOpenGL.so
    libX11-xcb.so
    libxkbcommon.so
    libSM.so
    libICE.so
    libglib-2.0.so
    libgobject-2.0.so
    libgio-2.0.so
    libgirepository-2.0.so
    libgmodule-2.0.so
    libgthread-2.0.so
    libdbus-1.so
    libpipewire-0.3.so
    libpulse.so
    libpulse-simple.so
    libpulse-mainloop-glib.so
    libupower-glib.so
    libudev.so
    libGrantlee_Templates.so
    libGrantlee_TextDocument.so
    liblo.so
    libtag_c.so
)
set(_appimage_shared_libraries)
foreach(_lib IN LISTS _appimage_buildenv_shared_libraries)
  get_filename_component(_lib_name "${_lib}" NAME)
  set(_excluded OFF)
  foreach(_syslib IN LISTS _appimage_system_libraries)
    # Match a leading substring (e.g. "libpulse.so" matches "libpulse.so.0.24.3").
    # Use string(FIND) rather than MATCHES to avoid regex metacharacters
    # like '+' in "libstdc++.so".
    string(FIND "${_lib_name}" "${_syslib}" _lib_pos)
    if(_lib_pos EQUAL 0)
      set(_excluded ON)
    endif()
  endforeach()
  if(NOT _excluded)
    list(APPEND _appimage_shared_libraries "${_lib}")
  endif()
endforeach()
if(_appimage_shared_libraries)
  install(FILES ${_appimage_shared_libraries} DESTINATION "${CMAKE_INSTALL_LIBDIR}")
endif()
unset(_appimage_buildenv_shared_libraries)
unset(_appimage_system_libraries)
unset(_appimage_shared_libraries)
unset(_appimage_lib_name)
unset(_appimage_excluded)
unset(_appimage_syslib)
