#!/bin/bash
set -o pipefail

COMMAND=$1
shift

realpath() {
    OLDPWD="${PWD}"
    cd "$1" || exit 1
    pwd
    cd "${OLDPWD}" || exit 1
}

MIXXX_ROOT="$(realpath "$(dirname "${BASH_SOURCE[0]}")/..")"

read_envname() {
    cat "${MIXXX_ROOT}/cmake/macos_build_environment_name"
}

[ -z "$BUILDENV_BASEPATH" ] && BUILDENV_BASEPATH="${MIXXX_ROOT}/buildenv"

case "$COMMAND" in
    name)
        envname="$(read_envname)"
        if [ "$1" = "--ghactions" ]
        then
            echo "::set-output name=buildenv_name::$envname"
        else
            echo "$envname"
        fi
        ;;

    setup)
        BUILDENV_NAME="$(read_envname)"

        if [[ "$BUILDENV_NAME" =~ .*macosminimum([0-9]*\.[0-9]*).* ]]
        then
            MACOSX_DEPLOYMENT_TARGET="${BASH_REMATCH[1]}"
        else
            echo "Build environment did not match expected pattern. Check cmake/macos_build_environment_name file." >&2
            exit 1
        fi

        BUILDENV_PATH="${BUILDENV_BASEPATH}/${BUILDENV_NAME}"
        mkdir -p "${BUILDENV_BASEPATH}"
        if [ ! -d "${BUILDENV_PATH}" ]
        then
            curl "https://downloads.mixxx.org/builds/buildserver/2.3.x-unix/${BUILDENV_NAME}.tar.gz" -o "${BUILDENV_PATH}.tar.gz"
            # TODO: verify download using sha256sum?
            tar xf "${BUILDENV_PATH}.tar.gz" -C "${BUILDENV_BASEPATH}"
            rm "${BUILDENV_PATH}.tar.gz"
        fi
        echo "Using build environment: ${BUILDENV_PATH}"

        CMAKE_PREFIX_PATH="${BUILDENV_PATH}"
        PATH="${BUILDENV_PATH}/bin:${PATH}"
        QT_PATH="$(find "${BUILDENV_PATH}" -type d -path "*/cmake/Qt5")"
        QT_QPA_PLATFORM_PLUGIN_PATH="$(find "${BUILDENV_PATH}" -type d -path "*/plugins")"

        echo "Environent Variables:"
        echo "- PATH=${PATH}"
        echo "- QT_QPA_PLATFORM_PLUGIN_PATH=${QT_QPA_PLATFORM_PLUGIN_PATH}"
        echo "- MACOSX_DEPLOYMENT_TARGET=${MACOSX_DEPLOYMENT_TARGET}"
        echo ""
        echo "CMake Configuration:"
        echo "- CMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}"
        echo "- Qt5_DIR=${QT_PATH}"

        if [ "$1" = "--ghactions" ]
        then
            echo "::set-output name=macosx_deployment_target::${MACOSX_DEPLOYMENT_TARGET}"
            echo "::set-output name=cmake_prefix_path::${CMAKE_PREFIX_PATH}"
            echo "::set-output name=path::${PATH}"
            echo "::set-output name=qt_path::${QT_PATH}"
            echo "::set-output name=qt_qpa_platform_plugin_path::${QT_QPA_PLATFORM_PLUGIN_PATH}"
        else
            export MACOSX_DEPLOYMENT_TARGET
            export CMAKE_PREFIX_PATH
            export PATH
            export QT_PATH
            export QT_QPA_PLATFORM_PLUGIN_PATH
        fi
        ;;
esac
