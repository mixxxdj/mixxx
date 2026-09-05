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
file(WRITE "${MIXXX_APPIMAGE_DESKTOP_FILE}" "${MIXXX_APPIMAGE_DESKTOP_CONTENT}")
install(
  FILES "${MIXXX_APPIMAGE_DESKTOP_FILE}"
  DESTINATION "${CMAKE_INSTALL_DATADIR}/applications"
  RENAME "mixxx-appimage.desktop"
)

# Install the runtime dependencies of the mixxx binary into AppDir/lib
# (the binary's RUNPATH is $ORIGIN/../lib, set by CPack, so bundled
# libraries must end up in the top-level lib/).
#
# The library set is determined dynamically from the actual dependencies of
# the mixxx binary at install time, so newly introduced dependencies are
# bundled automatically without maintaining a per-library list.  Libraries
# that must be provided by the host system are excluded, based on the
# official AppImage excludelist (see linuxdeploy, which maintains it):
# https://github.com/probonopd/AppImages/blob/master/excludelist
# These are the C library and compiler runtime, GPU driver libraries, the
# core X11 client libraries (which must match the running X server), and
# sound server client libraries (which must match the local server).

# Collect the runtime dependency set from the mixxx binary.  A target's
# dependency set can only be populated by an install(TARGETS) call, so mixxx
# is installed a second time here (to the same destination as the generic
# rule above, which is a harmless idempotent copy).
install(
  TARGETS mixxx
  # The RUNTIME_DEPENDENCY_SET keyword must precede the artifact options
  # (RUNTIME/BUNDLE DESTINATION), or native install() rejects it.
  RUNTIME_DEPENDENCY_SET mixxx_runtime_deps
  RUNTIME DESTINATION "${MIXXX_INSTALL_BINDIR}"
  BUNDLE DESTINATION .
)
install(
  RUNTIME_DEPENDENCY_SET
  mixxx_runtime_deps
  # The resolution options (DIRECTORIES, PRE_EXCLUDE_REGEXES, ...) must
  # precede the artifact group (LIBRARY DESTINATION), or install() rejects
  # them as unknown arguments.
  DIRECTORIES "${MIXXX_VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/lib"
  PRE_EXCLUDE_REGEXES
    # glibc and the compiler runtime
    "^ld-linux.*"
    "^libc\\.so.*"
    "^libm\\.so.*"
    "^libdl\\.so.*"
    "^libpthread\\.so.*"
    "^librt\\.so.*"
    "^libresolv\\.so.*"
    "^libutil\\.so.*"
    "^libnss_.*"
    "^libmvec\\.so.*"
    "^libanl\\.so.*"
    "^libthread_db\\.so.*"
    "^libcidn\\.so.*"
    "^libBrokenLocale\\.so.*"
    "^libstdc\\+\\+\\.so.*"
    "^libgcc_s\\.so.*"
    # GPU / display driver stack
    "^libGL\\.so.*"
    "^libEGL\\.so.*"
    "^libGLX\\.so.*"
    "^libOpenGL\\.so.*"
    "^libGLdispatch\\.so.*"
    "^libdrm\\.so.*"
    "^libglapi\\.so.*"
    "^libgbm\\.so.*"
    # Core X11 client libraries (must match the running X server)
    "^libX11\\.so.*"
    "^libX11-xcb\\.so.*"
    "^libxcb\\.so.*"
    "^libxcb-dri2\\.so.*"
    "^libxcb-dri3\\.so.*"
    "^libwayland-client\\.so.*"
    # Sound server client libraries (must match the local sound server)
    "^libasound\\.so.*"
    "^libjack\\.so.*"
    "^libpipewire.*"
    # Low-level font stack and other system essentials
    "^libfontconfig\\.so.*"
    "^libfreetype\\.so.*"
    "^libharfbuzz\\.so.*"
    "^libICE\\.so.*"
    "^libSM\\.so.*"
    "^libuuid\\.so.*"
    "^libz\\.so.*"
    "^libexpat\\.so.*"
    "^libcom_err\\.so.*"
    "^libgpg-error\\.so.*"
    "^libusb-1\\.0\\.so.*"
    "^libgmp\\.so.*"
  LIBRARY
  DESTINATION
  "${CMAKE_INSTALL_LIBDIR}"
)
