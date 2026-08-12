#!/usr/bin/env bash
# Download appimagetool. AppImages run directly via FUSE (libfuse2t64), so no
# extraction is needed.
#
# Usage: install-appimagetool.sh [arch]   (arch defaults to uname -m)
set -euo pipefail

ARCH="${1:-$(uname -m)}"

case "${ARCH}" in
  x86_64)  APPIMAGE_ARCH="x86_64"  ;;
  aarch64) APPIMAGE_ARCH="aarch64" ;;
  *)
    echo "Error: unsupported architecture: ${ARCH}" >&2
    exit 1
    ;;
esac

URL="https://github.com/AppImage/appimagetool/releases/download/continuous/appimagetool-${APPIMAGE_ARCH}.AppImage"

echo "Downloading appimagetool for ${APPIMAGE_ARCH}..."
curl -fsSL --connect-timeout 15 --max-time 120 \
  -o /opt/appimagetool.AppImage "${URL}"
chmod +x /opt/appimagetool.AppImage

echo "appimagetool installed to /opt/appimagetool.AppImage"