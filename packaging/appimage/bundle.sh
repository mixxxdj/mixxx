#!/bin/bash
# ============================================================
# Mixxx AppImage Dependency Bundler
# ============================================================
# Scans binaries in the AppDir, copies all needed shared
# libraries, Qt plugins, and QML modules into the AppDir.
#
# Usage: bundle.sh /path/to/AppDir
# ============================================================
set -euo pipefail

APPDIR="$1"

if [ ! -d "$APPDIR" ]; then
    echo "ERROR: AppDir not found: $APPDIR"
    exit 1
fi

echo "Bundling dependencies into: $APPDIR"

# ---- Detect architecture and system paths ----
ARCH=$(uname -m)
case "$ARCH" in
    x86_64)  MULTIARCH="x86_64-linux-gnu"  ;;
    aarch64) MULTIARCH="aarch64-linux-gnu" ;;
    armv7l)  MULTIARCH="arm-linux-gnueabihf" ;;
    *)
        echo "ERROR: Unsupported architecture: $ARCH"
        exit 1
        ;;
esac

SYS_LIB_DIR="/usr/lib/${MULTIARCH}"
SYS_QT_PLUGINS_DIR="${SYS_LIB_DIR}/qt6/plugins"
SYS_QT_QML_DIR="${SYS_LIB_DIR}/qt6/qml"

APPDIR_LIB_DIR="${APPDIR}/usr/lib"
APPDIR_PLUGINS_DIR="${APPDIR}/usr/plugins"
APPDIR_QML_DIR="${APPDIR}/usr/qml"

mkdir -p "$APPDIR_LIB_DIR" "$APPDIR_PLUGINS_DIR" "$APPDIR_QML_DIR"

# ---- Libraries to exclude (standard system libs) ----
# These are provided by the host system's glibc and libstdc++.
# Since the build environment (Debian 12) matches the host (Debian 12),
# they do not need to be bundled.
EXCLUDE_LIBS=(
    "ld-linux"
    "ld-linux-aarch64"
    "libc.so"
    "libc-"
    "libcrypt.so"
    "libcrypt-"
    "libdl.so"
    "libdl-"
    "libm.so"
    "libm-"
    "libpthread.so"
    "libpthread-"
    "librt.so"
    "librt-"
    "libresolv.so"
    "libresolv-"
    "libutil.so"
    "libutil-"
    "libnss_"
    "libnsl.so"
    "libanl.so"
    "libBrokenLocale.so"
    "libcidn.so"
    "libthread_db.so"
    "libgcc_s.so"
    "libstdc++.so.6"
    # OpenGL/GLX/EGL — provided by host system Mesa
    "libGLX"
    "libEGL"
    "libOpenGL"
    "libGLESv1_CM"
    "libGLESv2"
    "libGLdispatch"
    "libGLX_mesa"
    "libEGL_mesa"
    "libGLX_indirect"
)

# ---- Helper: Check if a library should be excluded ----
should_exclude() {
    local lib="$1"
    local basename
    basename=$(basename "$lib")
    for excl in "${EXCLUDE_LIBS[@]}"; do
        if echo "$basename" | grep -q "^${excl}"; then
            return 0
        fi
    done
    return 1
}

# ---- Helper: Resolve symlinks to real file ----
resolve_library() {
    local path="$1"
    local realpath
    realpath=$(readlink -f "$path" 2>/dev/null || echo "$path")
    echo "$realpath"
}

# ---- Helper: Copy a library to AppDir ----
copy_library() {
    local src="$1"
    local dest_dir="$2"

    if [ ! -f "$src" ]; then
        return 1
    fi

    local filename
    filename=$(basename "$src")

    # Skip if already exists
    if [ -f "${dest_dir}/${filename}" ]; then
        return 0
    fi

    # Skip excluded libraries
    if should_exclude "$src"; then
        return 0
    fi

    # Copy the library
    cp -L "$src" "${dest_dir}/${filename}" 2>/dev/null || return 1
    chmod 644 "${dest_dir}/${filename}" 2>/dev/null || true

    echo "  Copied: ${filename}"
    return 0
}

# ---- Step 1: Scan all ELF binaries in AppDir ----
echo ""
echo "Step 1: Scanning binaries in AppDir..."
ELF_FILES=()
while IFS= read -r -d '' f; do
    if file "$f" | grep -qE "ELF.*(executable|shared object)"; then
        ELF_FILES+=("$f")
    fi
done < <(find "$APPDIR" -type f \( -executable -o -name "*.so*" \) -print0 2>/dev/null)

echo "Found ${#ELF_FILES[@]} ELF files to scan"

# ---- Step 2: Collect all needed libraries ----
echo ""
echo "Step 2: Resolving library dependencies..."

# Use a temp file to track processed libraries (avoid infinite loops)
PROCESSED_LIBS=$(mktemp)
NEEDED_LIBS=$(mktemp)

# Initial scan of all ELF files
for elf in "${ELF_FILES[@]}"; do
    ldd "$elf" 2>/dev/null | grep -E "=> /" | awk '{print $3}' >> "$NEEDED_LIBS" || true
done

# Sort and deduplicate
sort -u "$NEEDED_LIBS" -o "$NEEDED_LIBS"

# Copy libraries, then recursively scan new ones
TOTAL_COPIED=0
ROUND=0
while [ "$(wc -l < "$NEEDED_LIBS")" -gt 0 ]; do
    ROUND=$((ROUND + 1))
    NEW_NEEDED=$(mktemp)
    CURRENT_BATCH=$(mktemp)
    mv "$NEEDED_LIBS" "$CURRENT_BATCH"

    while IFS= read -r libpath; do
        [ -z "$libpath" ] && continue

        # Resolve symlinks
        reallib=$(resolve_library "$libpath")
        soname=$(basename "$reallib")

        # Skip if already processed
        if grep -q "^${soname}$" "$PROCESSED_LIBS" 2>/dev/null; then
            continue
        fi
        echo "$soname" >> "$PROCESSED_LIBS"

        # Copy the library
        if copy_library "$reallib" "$APPDIR_LIB_DIR"; then
            TOTAL_COPIED=$((TOTAL_COPIED + 1))
        fi

        # Scan this library's dependencies
        ldd "$reallib" 2>/dev/null | grep -E "=> /" | awk '{print $3}' >> "$NEW_NEEDED" || true
    done < "$CURRENT_BATCH"

    # Sort and deduplicate
    sort -u "$NEW_NEEDED" -o "$NEW_NEEDED" 2>/dev/null || true
    mv "$NEW_NEEDED" "$NEEDED_LIBS"
    rm -f "$CURRENT_BATCH"

    echo "  Round ${ROUND}: ${TOTAL_COPIED} libraries copied so far"

    # Safety limit
    if [ "$ROUND" -ge 10 ]; then
        echo "  Warning: Reached recursion limit, some libraries may be missing"
        break
    fi
done

rm -f "$PROCESSED_LIBS" "$NEEDED_LIBS"

# ---- Step 3: Copy Qt plugins ----
echo ""
echo "Step 3: Copying Qt plugins..."

if [ -d "$SYS_QT_PLUGINS_DIR" ]; then
    # Copy platform plugins (essential for Qt GUI)
    for plugdir in platforms imageformats styles audio multimedia sqldrivers; do
        src="${SYS_QT_PLUGINS_DIR}/${plugdir}"
        if [ -d "$src" ]; then
            mkdir -p "${APPDIR_PLUGINS_DIR}/${plugdir}"
            cp -rL "$src"/*.so "${APPDIR_PLUGINS_DIR}/${plugdir}/" 2>/dev/null || true
            echo "  Copied Qt plugin: ${plugdir}"
        fi
    done
else
    echo "  Warning: Qt plugins directory not found at ${SYS_QT_PLUGINS_DIR}"
fi

# ---- Step 4: Copy QML modules ----
echo ""
echo "Step 4: Copying QML modules..."

if [ -d "$SYS_QT_QML_DIR" ]; then
    # Copy the entire QML directory (includes all needed modules)
    cp -rL "$SYS_QT_QML_DIR"/* "$APPDIR_QML_DIR/" 2>/dev/null || true
    echo "  Copied QML modules from ${SYS_QT_QML_DIR}"
else
    echo "  Warning: QML directory not found at ${SYS_QT_QML_DIR}"
fi

# ---- Step 5: Copy additional libraries (audio backends, etc.) ----
echo ""
echo "Step 5: Copying additional libraries..."

# Find and copy libraries that might not be direct deps of ELF files
# but are needed at runtime (dlopen'd)
ADDITIONAL_LIBS=(
    "libpulsecommon"    # PulseAudio
    "libpulse"           # PulseAudio
    "libpipewire"        # PipeWire
    "libjack"            # JACK
    "libasound"          # ALSA
    "libva"              # VA-API
    "libvdpau"           # VDPAU
    "libxcb"             # X11
    "libxcb-cursor"      # X11 cursor (needed by Qt6 xcb plugin)
    "libxcb-icccm"       # X11 ICCCM
    "libxcb-image"       # X11 image
    "libxcb-keysyms"     # X11 keysyms
    "libxcb-randr"       # X11 randr
    "libxcb-render-util" # X11 render util
    "libxcb-shape"       # X11 shape
    "libxcb-sync"        # X11 sync
    "libxcb-xfixes"      # X11 xfixes
    "libxcb-xkb"         # X11 xkb
    "libxcb-xinput"      # X11 xinput
    "libSM"              # Session Management
    "libICE"             # Inter-Client Exchange
    "libxkbcommon-x11"   # XKB common X11
    "libX11"             # X11
    "libXext"            # X11
    "libQt6XcbQpa"       # Qt6 xcb platform plugin support
    "libxkbcommon"       # XKB
    "libfontconfig"      # Fonts
    "libfreetype"        # Fonts
    "libharfbuzz"        # Text shaping
    "libpcre2"           # PCRE2 (Qt dependency)
    "libdouble-conversion"  # Qt dependency
    "libmd4c"            # Qt dependency
    "libinput"           # Input
    "libudev"            # udev
    "libgbm"             # GBM
    "libdrm"             # DRM
    "libthai"            # Thai language support
    "libdatrie"          # Trie library
    "libfribidi"         # BiDi text
    "libxml2"            # XML
    "libxslt"            # XSLT
    "libssl"             # OpenSSL
    "libcrypto"          # OpenSSL
    "libsasl2"           # SASL
    "libjson-c"          # JSON
    "liblzma"            # XZ
    "libzstd"            # Zstd
    "libbz2"             # Bzip2
    "libicu"             # ICU (Qt dependency)
    "libpng"             # PNG
    "libjpeg"            # JPEG
    "libtiff"            # TIFF
    "libwebp"            # WebP
    "libgthread"         # GLib threads
    "libglib"            # GLib
    "libgobject"         # GObject
    "libgio"             # GIO
    "libgmodule"         # GModule
    "libmtdev"           # MT dev
    "liblilv"            # LV2
    "libserd"            # LV2
    "libsord"            # LV2
    "libsratom"          # LV2
    "libzix"             # LV2
    "liblv2"             # LV2
    "libprotobuf"        # Protocol Buffers
    "libupower"          # UPower
    "libchromaprint"     # Chromaprint
    "libebur128"         # EBU R128
    "libshout"           # SHOUTcast
    "libsoundtouch"      # SoundTouch
    "librubberband"      # Rubber Band
    "libid3tag"          # ID3 tag
    "libmad"             # MAD (MP3)
    "libmodplug"         # ModPlug
    "libfaad"            # AAC
    "libmp3lame"         # LAME
    "libopus"            # Opus
    "libopusfile"        # Opus file
    "libwavpack"         # WavPack
    "libfftw3"           # FFTW
    "libavcodec"         # FFmpeg
    "libavformat"        # FFmpeg
    "libavutil"          # FFmpeg
    "libswresample"      # FFmpeg
    "libtag"             # TagLib
    "libFLAC"            # FLAC
    "libvorbis"          # Vorbis
    "libogg"             # Ogg
    "libsndfile"         # sndfile
    "libportaudio"       # PortAudio
    "libportmidi"        # PortMidi
    "libhidapi"          # HID API
    "libusb"             # USB
    "libqt6keychain"     # QtKeychain
)

for libpat in "${ADDITIONAL_LIBS[@]}"; do
    # Find the library file (try multiple locations)
    libfile=$(find "$SYS_LIB_DIR" "/usr/lib" -maxdepth 2 -name "${libpat}*.so*" -type f 2>/dev/null | head -1)
    if [ -n "$libfile" ]; then
        copy_library "$libfile" "$APPDIR_LIB_DIR"
    fi
done

# ---- Step 6: Copy NSS libraries ----
echo ""
echo "Step 6: Copying NSS libraries..."
# NSS libraries are needed by glibc for DNS/user resolution at runtime.
# They are NOT excluded by EXCLUDE_LIBS (exclusion is by basename match,
# and libnss_* is not in the list).
find /lib /usr/lib -name "libnss_*" -type f 2>/dev/null | while read -r nsslib; do
    copy_library "$nsslib" "$APPDIR_LIB_DIR"
done

# ---- Step 7: Copy fontconfig cache and fonts ----
echo ""
echo "Step 7: Copying fonts..."

# Copy fonts that are installed
if [ -d "/usr/share/fonts" ]; then
    mkdir -p "${APPDIR}/usr/share/fonts"
    cp -rL /usr/share/fonts/* "${APPDIR}/usr/share/fonts/" 2>/dev/null || true
    echo "  Copied fonts"
fi

# Copy fontconfig configuration
if [ -d "/etc/fonts" ]; then
    mkdir -p "${APPDIR}/etc"
    cp -rL /etc/fonts "${APPDIR}/etc/" 2>/dev/null || true
    echo "  Copied fontconfig"
fi

# ---- Step 8: Create ldconfig cache ----
echo ""
echo "Step 8: Creating ldconfig cache..."
ldconfig -r "$APPDIR" -X 2>/dev/null || true

# ---- Step 9: Create qt.conf ----
echo ""
echo "Step 9: Creating qt.conf..."
cat > "${APPDIR}/usr/bin/qt.conf" << 'QTCONF'
[Paths]
Plugins = ../plugins
Imports = ../qml
Qml2Imports = ../qml
QTCONF
echo "  Created qt.conf"

# ---- Step 10: Create SONAME symlinks ----
echo ""
echo "Step 10: Creating SONAME symlinks..."
# The copy_library function uses cp -L which follows symlinks, so SONAME
# symlinks (e.g. libavcodec.so.61 -> libavcodec.so.61.19.101) are lost.
# Recreate them from the DT_SONAME tag in each library.
find "$APPDIR_LIB_DIR" -name "*.so*" -type f 2>/dev/null | while read -r lib; do
    soname=$(objdump -p "$lib" 2>/dev/null | grep "SONAME" | awk '{print $2}')
    if [ -n "$soname" ] && [ "$soname" != "$(basename "$lib")" ]; then
        linkpath="${APPDIR_LIB_DIR}/${soname}"
        if [ ! -f "$linkpath" ] && [ ! -L "$linkpath" ]; then
            ln -sf "$(basename "$lib")" "$linkpath"
            echo "  Symlink: ${soname} -> $(basename "$lib")"
        fi
    fi
done

# ---- Step 11: Clean up ----
echo ""
echo "Step 11: Cleaning up..."
# Remove any stray symlinks that point outside the AppDir
find "$APPDIR_LIB_DIR" -type l ! -exec test -e {} \; -delete 2>/dev/null || true
# Remove static libraries
find "$APPDIR" -name "*.a" -type f -delete 2>/dev/null || true
# Remove debugging symbols (optional, reduces size)
find "$APPDIR" -name "*.debug" -type f -delete 2>/dev/null || true

echo ""
echo "=== Dependency bundling complete ==="
echo "Total libraries copied: ${TOTAL_COPIED}"

# Count final results
echo "Libraries in AppDir: $(find "$APPDIR_LIB_DIR" -name "*.so*" -type f 2>/dev/null | wc -l)"
echo "QML modules in AppDir: $(find "$APPDIR_QML_DIR" -name "*.qmltypes" -type f 2>/dev/null | wc -l || true)"
echo "Qt plugins in AppDir: $(find "$APPDIR_PLUGINS_DIR" -name "*.so" -type f 2>/dev/null | wc -l || true)"
echo ""
