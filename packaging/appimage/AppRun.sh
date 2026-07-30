#!/bin/bash
# ============================================================
# Mixxx AppImage Entry Point (AppRun)
# ============================================================
set -euo pipefail

# Get the AppDir root (where this script lives)
APPDIR="$(dirname "$(readlink -f "$0")")"

# Set up library and plugin paths
export LD_LIBRARY_PATH="${APPDIR}/usr/lib:${LD_LIBRARY_PATH:-}"
export QT_PLUGIN_PATH="${APPDIR}/usr/plugins"
export QML2_IMPORT_PATH="${APPDIR}/usr/qml"
export QT_QPA_PLATFORM_PLUGIN_PATH="${APPDIR}/usr/plugins/platforms"
export PATH="${APPDIR}/usr/bin:${PATH:-}"

# Set XDG data dirs so the app can find its data
export XDG_DATA_DIRS="${APPDIR}/usr/share:${XDG_DATA_DIRS:-}"

# Set HOME if not set (some minimal environments)
if [ -z "${HOME:-}" ]; then
    HOME="${XDG_CONFIG_HOME:-$HOME}"
fi
if [ -z "${HOME:-}" ]; then
    HOME="${APPDIR}/../.mixxx-home"
    mkdir -p "$HOME" 2>/dev/null || true
fi

# Find the mixxx binary
MIXXX_BIN="${APPDIR}/usr/bin/mixxx"
if [ ! -x "$MIXXX_BIN" ]; then
    echo "ERROR: Mixxx binary not found at ${MIXXX_BIN}"
    exit 1
fi

# Execute mixxx (glibc matches host system, so no bundled ld-linux needed)
if command -v pasuspender &>/dev/null; then
    pasuspender -- "$MIXXX_BIN" "$@" 2>/dev/null || true
fi
exec "$MIXXX_BIN" "$@"