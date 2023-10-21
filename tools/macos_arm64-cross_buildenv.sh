#!/bin/bash
tools_path=$(dirname "$0")
BUILDENV_ARM64=TRUE "${tools_path}/macos_buildenv.sh" "$@"
