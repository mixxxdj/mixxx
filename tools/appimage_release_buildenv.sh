#!/bin/bash

if [ -z "${GITHUB_ENV}" ]; then
  echo "This script is currently intended for CI use only (source tools/appimage_buildenv.sh instead)"
  exit 1
fi

tools_path=$(dirname "$0")
BUILDENV_RELEASE=TRUE "${tools_path}/appimage_buildenv.sh" "$@"
