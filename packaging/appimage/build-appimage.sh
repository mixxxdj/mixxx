#!/usr/bin/env bash
# Build the Mixxx AppImage from a configured CMake build directory.
# Usage: build-appimage.sh <build_dir> <arch> [appimagetool_path]
set -euo pipefail

BUILD_DIR="$(cd "$1" && pwd)"
ARCH="$2"
APPIMAGETOOL="${3:-/opt/appimagetool.AppImage}"

# Derive the vcpkg target triplet from the build architecture.  This is used
# as a fallback when VCPKG_TARGET_TRIPLET is not set by the buildenv script.
case "${ARCH:-$(uname -m)}" in
    x86_64)  DEFAULT_TRIPLET="x64-linux"    ; MULTIARCH="x86_64-linux-gnu"  ;;
    aarch64) DEFAULT_TRIPLET="arm64-linux"  ; MULTIARCH="aarch64-linux-gnu" ;;
    *)       DEFAULT_TRIPLET="x64-linux"    ; MULTIARCH="x86_64-linux-gnu"  ;;
esac

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
STAGING_DIR="${BUILD_DIR}/staging"
APPDIR="${STAGING_DIR}"

# Install to staging AppDir
mkdir -p "${APPDIR}"
DESTDIR="${APPDIR}" cmake --build "${BUILD_DIR}" --target install

# Copy the AppRun entry point to the AppDir root
cp "${SCRIPT_DIR}/AppRun.sh" "${APPDIR}/AppRun"
chmod +x "${APPDIR}/AppRun"

# Copy the CMake-generated desktop file to the AppDir root
DESKTOP_SRC="${APPDIR}/usr/share/applications/mixxx-appimage.desktop"
if [ -f "${DESKTOP_SRC}" ]; then
  cp "${DESKTOP_SRC}" "${APPDIR}/mixxx-appimage.desktop"
else
  echo "Error: desktop file not found at ${DESKTOP_SRC}" >&2
  exit 1
fi

# Copy icon to AppDir root (appimagetool needs .DirIcon and mixxx.png)
ICON=$(find "${APPDIR}" -name "mixxx.png" -path "*256x256*" 2>/dev/null | head -1)
if [ -z "${ICON}" ]; then
  ICON=$(find "${APPDIR}" -name "mixxx.png" 2>/dev/null | head -1)
fi
if [ -n "${ICON}" ]; then
  cp "${ICON}" "${APPDIR}/.DirIcon"
  cp "${ICON}" "${APPDIR}/mixxx.png"
else
  echo "Warning: no mixxx.png icon found in staging" >&2
fi

# Strip debug symbols (static linking packs all deps into one binary)
if [ -f "${APPDIR}/usr/bin/mixxx" ]; then
  strip --strip-unneeded "${APPDIR}/usr/bin/mixxx"
fi

# Bundle host system libraries. appimagetool only packages what's in the
# AppDir, so copy the real files (cp -L resolves symlinks) from the system.
#
# Libraries NEVER bundled (must come from the host system):
#   glibc core, GCC runtimes, Mesa/OpenGL, systemd, libdbus-1
#
# GLib family IS bundled from the build host to avoid a constructor-ordering
# issue on Debian 13/trixie where g_type_plugin_get_type runs before
# gobject_init_ctor, leaving static_quark_type_flags NULL.
SKIP_LIB_REGEX="^(libc\.so|ld-linux|libm\.so|libmvec|libpthread|libdl\.so|librt\.so|libresolv|libutil|libnss_|libcrypt|libnsl|libanl|libBrokenLocale|libthread_db|libgcc_s|libstdc\+\+|libGLX|libEGL|libOpenGL|libGLdispatch|libGLESv1_CM|libGLESv2|libGL\.so|libsystemd|libdbus-1)"

# Recursively scan dependencies until nothing new is copied, so deep
# transitive deps (libs-of-libs) are bundled too.
if [ -f "${APPDIR}/usr/bin/mixxx" ]; then
  mkdir -p "${APPDIR}/usr/lib"
  for round in 1 2 3 4 5 6 7 8 9 10; do
    # Collect deps of the mixxx binary plus every lib already copied so far.
    needed=$(
      {
        ldd "${APPDIR}/usr/bin/mixxx" 2>/dev/null
        find "${APPDIR}/usr/lib" -maxdepth 1 -type f -name "*.so*" -print0 2>/dev/null \
          | xargs -0 -r -n1 ldd 2>/dev/null
      } | sed -nE 's/.*=> ([^ ]+) \(0x[0-9a-f]+\)/\1/p' | sort -u
    )

    [ -z "$needed" ] && break
    copied=0
    while IFS= read -r lib; do
      [ -z "$lib" ] && continue
      base=$(basename "$lib")
      target="${APPDIR}/usr/lib/${base}"
      [ -f "$target" ] && continue
      echo "$base" | grep -Eq "${SKIP_LIB_REGEX}" && continue
      reallib=$(readlink -f "$lib" 2>/dev/null || echo "$lib")
      cp -L "$reallib" "$target" 2>/dev/null && copied=$((copied + 1))
    done <<EOF
${needed}
EOF
    [ "$copied" -eq 0 ] && break
  done

  # Recreate SONAME symlinks: cp -L flattens the versioned symlink chain
  # (e.g. libfoo.so.1 -> libfoo.so.1.2.3), which the loader needs.
  find "${APPDIR}/usr/lib" -maxdepth 1 -type f -name "*.so*" 2>/dev/null | while read -r lib; do
    soname=$(objdump -p "$lib" 2>/dev/null | sed -n 's/.*SONAME *//p')
    if [ -n "$soname" ] && [ "$soname" != "$(basename "$lib")" ] && [ ! -e "${APPDIR}/usr/lib/${soname}" ]; then
      ln -s "$(basename "$lib")" "${APPDIR}/usr/lib/${soname}" 2>/dev/null || true
    fi
  done

  # Strip debug symbols from bundled libraries (strip preserves .dynamic headers)
  find "${APPDIR}/usr/lib" -maxdepth 1 -type f -name "*.so*" -exec strip --strip-unneeded {} \; 2>/dev/null || true

  # Copy Qt plugins and QML modules so the Qt runtime can find its platform /
  # image-format / multipmedia plugins and the QML modules. These must come
  # from the vcpkg buildenv so the bundled plugins match the buildenv's Qt6
  # runtime — never fall back to a system Qt6, which would risk ABI mismatch.
  SYS_QT_PLUGINS=""
  SYS_QT_QML=""
  if [ -n "${MIXXX_VCPKG_ROOT:-}" ]; then
    for p in \
      "${MIXXX_VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET:-${DEFAULT_TRIPLET}}/plugins" \
      "${MIXXX_VCPKG_ROOT}/plugins"; do
      if [ -d "$p" ]; then SYS_QT_PLUGINS="$p"; break; fi
    done
    for q in \
      "${MIXXX_VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET:-${DEFAULT_TRIPLET}}/qml" \
      "${MIXXX_VCPKG_ROOT}/qml"; do
      if [ -d "$q" ]; then SYS_QT_QML="$q"; break; fi
    done
  fi
  if [ -z "$SYS_QT_PLUGINS" ]; then
    echo "Warning: no Qt plugins found in buildenv (${MIXXX_VCPKG_ROOT:-unset})" >&2
  fi
  if [ -z "$SYS_QT_QML" ]; then
    echo "Warning: no QML modules found in buildenv (${MIXXX_VCPKG_ROOT:-unset})" >&2
  fi
  # Fall back to the system Qt6 plugins/QML so the AppImage carries its own
  # platform/image-format/media plugins even when the buildenv omits them.
  if [ -z "$SYS_QT_PLUGINS" ] && [ -d "/usr/lib/${MULTIARCH}/qt6/plugins" ]; then
    SYS_QT_PLUGINS="/usr/lib/${MULTIARCH}/qt6/plugins"
    echo "Falling back to system Qt plugins: ${SYS_QT_PLUGINS}" >&2
  fi
  if [ -z "$SYS_QT_QML" ] && [ -d "/usr/lib/${MULTIARCH}/qt6/qml" ]; then
    SYS_QT_QML="/usr/lib/${MULTIARCH}/qt6/qml"
    echo "Falling back to system QML modules: ${SYS_QT_QML}" >&2
  fi

  if [ -n "$SYS_QT_PLUGINS" ]; then
    mkdir -p "${APPDIR}/usr/plugins"
    for plugin in platforms imageformats styles audio multimedia sqldrivers iconengines; do
      if [ -d "$SYS_QT_PLUGINS/$plugin" ]; then
        cp -rL "$SYS_QT_PLUGINS/$plugin" "${APPDIR}/usr/plugins/"
      else
        echo "Warning: Qt plugin subdir '${plugin}' not found in ${SYS_QT_PLUGINS}" >&2
      fi
    done
    echo "Copied Qt plugins from ${SYS_QT_PLUGINS}"
  fi
  if [ -n "$SYS_QT_QML" ]; then
    mkdir -p "${APPDIR}/usr/qml"
    cp -rL "$SYS_QT_QML"/* "${APPDIR}/usr/qml/" 2>/dev/null || true
    echo "Copied QML modules from ${SYS_QT_QML}"
  fi

  # qt.conf tells Qt where the bundled plugins and QML live.
  cat > "${APPDIR}/usr/bin/qt.conf" << 'QTCONF'
[Paths]
Plugins = ../plugins
Imports = ../qml
Qml2Imports = ../qml
QTCONF

  ldconfig -r "${APPDIR}" 2>/dev/null || true
fi

# Run appimagetool
"${APPIMAGETOOL}" -v "${APPDIR}" "${BUILD_DIR}/Mixxx-${ARCH}.AppImage"

# Verify output
if [ ! -f "${BUILD_DIR}/Mixxx-${ARCH}.AppImage" ]; then
  echo "Error: AppImage was not created" >&2
  exit 1
fi
ls -lh "${BUILD_DIR}/Mixxx-${ARCH}.AppImage"