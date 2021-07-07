#!/bin/bash
set -o pipefail

# shellcheck disable=SC2091
if [ -z "${GITHUB_ENV}" ] && ! $(return 0 2>/dev/null); then
  echo "This script must be run by sourcing it:"
  echo "source $0 $*"
  exit 1
fi

realpath() {
    OLDPWD="${PWD}"
    cd "$1" || exit 1
    pwd
    cd "${OLDPWD}" || exit 1
}

# some hackery is required to be compatible with both bash and zsh
THIS_SCRIPT_NAME=${BASH_SOURCE[0]}
[ -z "$THIS_SCRIPT_NAME" ] && THIS_SCRIPT_NAME=$0

MIXXX_ROOT="$(realpath "$(dirname "$THIS_SCRIPT_NAME")/..")"

read -r -d'\n' BUILDENV_NAME BUILDENV_SHA256 < "${MIXXX_ROOT}/packaging/macos/build_environment"

[ -z "$BUILDENV_BASEPATH" ] && BUILDENV_BASEPATH="${MIXXX_ROOT}/buildenv"

case "$1" in
    name)
        if [ -n "${GITHUB_ENV}" ]; then
            echo "BUILDENV_NAME=$BUILDENV_NAME" >> "${GITHUB_ENV}"
        else
            echo "$BUILDENV_NAME"
        fi
        ;;

    setup)
        BUILDENV_PATH="${BUILDENV_BASEPATH}/${BUILDENV_NAME}"
        mkdir -p "${BUILDENV_BASEPATH}"
        if [ ! -d "${BUILDENV_PATH}" ]; then
            if [ "$1" != "--profile" ]; then
                echo "Build environment $BUILDENV_NAME not found in mixxx repository, downloading it..."
                curl "https://downloads.mixxx.org/dependencies/2.4/macOS/${BUILDENV_NAME}.zip" -o "${BUILDENV_PATH}.zip"
                OBSERVED_SHA256=$(shasum -a 256 "${BUILDENV_PATH}.zip"|cut -f 1 -d' ')
                if [[ "$OBSERVED_SHA256" == "$BUILDENV_SHA256" ]]; then
                    echo "Download matched expected SHA256 sum $BUILDENV_SHA256"
                else
                    echo "ERROR: Download did not match expected SHA256 checksum!"
                    echo "Expected $BUILDENV_SHA256"
                    echo "Got $OBSERVED_SHA256"
                    exit 1
                fi
                echo ""
                echo "Extracting ${BUILDENV_NAME}.tar.gz..."
                unzip "${BUILDENV_PATH}.zip" -d "${BUILDENV_BASEPATH}" && \
                echo "Successfully extracted ${BUILDENV_NAME}.tar.gz"
            else
                echo "Build environment $BUILDENV_NAME not found in mixxx repository, run the command below to download it."
                echo "source ${THIS_SCRIPT_NAME} setup"
                return # exit would quit the shell being started
            fi
        elif [ "$1" != "--profile" ]; then
            echo "Build environment found: ${BUILDENV_PATH}"
        fi

        export SDKROOT="${BUILDENV_BASEPATH}/MacOSX10.13.sdk"
        if [ -d "${SDKROOT}" ]; then
            if [ "$1" != "--profile" ]; then
                echo "macOS 10.13 SDK found: ${SDKROOT}"
            fi
        else
            echo "macOS 10.13 SDK not found, downloading it..."
            curl -L "https://github.com/phracker/MacOSX-SDKs/releases/download/10.15/MacOSX10.13.sdk.tar.xz" -o "${SDKROOT}.tar.xz"
            OBSERVED_SHA256=$(shasum -a 256 "${SDKROOT}.tar.xz"|cut -f 1 -d' ')
            EXPECTED_SHA256="a3a077385205039a7c6f9e2c98ecdf2a720b2a819da715e03e0630c75782c1e4"
            if [[ "$OBSERVED_SHA256" == "$EXPECTED_SHA256" ]]; then
                echo "Download matched expected SHA256 sum $EXPECTED_SHA256"
            else
                echo "ERROR: Download did not match expected SHA256 checksum!"
                echo "Expected $EXPECTED_SHA256"
                echo "Got $OBSERVED_SHA256"
                exit 1
            fi
            echo "Extracting MacOSX10.13.sdk.tar.xz..."
            tar xf "${SDKROOT}.tar.xz" -C "${BUILDENV_BASEPATH}" && \
            echo "Successfully extacted MacOSX10.13.sdk.tar.xz"
            rm "${SDKROOT}.tar.xz"
        fi

        export VCPKG_ROOT="${BUILDENV_PATH}"
        export VCPKG_OVERLAY_TRIPLETS="${BUILDENV_PATH}/overlay/triplets"
        export VCPKG_DEFAULT_TRIPLET=x64-osx
        export X_VCPKG_APPLOCAL_DEPS_INSTALL=ON

        echo_exported_variables() {
            echo "VCPKG_ROOT=${VCPKG_ROOT}"
            echo "VCPKG_OVERLAY_TRIPLETS=${VCPKG_OVERLAY_TRIPLETS}"
            echo "VCPKG_DEFAULT_TRIPLET=${VCPKG_DEFAULT_TRIPLET}"
            echo "X_VCPKG_APPLOCAL_DEPS_INSTALL=${X_VCPKG_APPLOCAL_DEPS_INSTALL}"
        }

        if [ -n "${GITHUB_ENV}" ]; then
            echo_exported_variables >> "${GITHUB_ENV}"
        elif [ "$1" != "--profile" ]; then
            echo ""
            echo "Exported environment variables:"
            echo_exported_variables
        fi
        ;;
    *)
        echo "Usage: source macos_buildenv.sh [options]"
        echo ""
        echo "options:"
        echo "   help       Displays this help."
        echo "   name       Displays the name of the required build environment."
        echo "   setup      Installs the build environment."
        ;;
esac
