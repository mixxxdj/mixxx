#!/bin/bash
set -o pipefail

COMMAND=$1
shift

echo "No build environment needed for Ubuntu, please install dependencies using apt." >&2

case "$COMMAND" in
    name)
        if [ "$1" = "--ghactions" ]
        then
            echo "::set-output name=buildenv_name::"
        fi
        ;;

    setup)
        if [ "$1" = "--ghactions" ]
        then
            QT_QPA_PLATFORM_PLUGIN_PATH="$(qtpaths --plugin-directory)"

            echo "::set-output name=buildenv_path::"
            echo "::set-output name=macosx_deployment_target::"
            echo "::set-output name=cmake_prefix_path::"
            echo "::set-output name=path::${PATH}"
            echo "::set-output name=qt_path::"
            echo "::set-output name=qt_qpa_platform_plugin_path::${QT_QPA_PLATFORM_PLUGIN_PATH}"
        fi
        ;;
esac
