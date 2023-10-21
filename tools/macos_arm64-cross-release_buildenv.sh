#!/bin/bash
tools_path=$(dirname "$0")
BUILDENV_ARM64=TRUE BUILDENV_RELEASE=TRUE "${tools_path}/macos_buildenv.sh" "$@"
