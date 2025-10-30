#!/bin/bash

if [ -z "${GITHUB_ENV}" ]; then
  echo "This script is currently intended for CI use only (source tools/macos_buildenv.sh instead)"
  exit 1
fi

tools_path=$(dirname "$0")
BUILDENV_ARM64_CROSS=TRUE "${tools_path}/macos_buildenv.sh" "$@"
