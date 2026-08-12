#!/bin/bash
# AppImage entry point (AppRun).
# Sets up the environment so the bundled libraries, Qt plugins, QML modules,
# and data files are found at runtime.
set -euo pipefail

APPDIR="$(dirname "$(readlink -f "$0")")"

# Library and Qt paths
export LD_LIBRARY_PATH="${APPDIR}/usr/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export QT_PLUGIN_PATH="${APPDIR}/usr/plugins"
export QML2_IMPORT_PATH="${APPDIR}/usr/qml"
export QT_QPA_PLATFORM_PLUGIN_PATH="${APPDIR}/usr/plugins/platforms"

# XDG data dirs
export XDG_DATA_DIRS="${APPDIR}/usr/share:${XDG_DATA_DIRS:-}"

# Set HOME if not set (minimal environments)
if [ -z "${HOME:-}" ]; then
  HOME="${APPDIR}/../.mixxx-home"
  mkdir -p "$HOME" 2>/dev/null || true
fi

exec "${APPDIR}/usr/bin/mixxx" "$@"