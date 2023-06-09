#!/bin/bash
tools_path=$(dirname "$0")
BUILDENV_RELEASE=TRUE "${tools_path}/macos_buildenv.sh" "$@"
