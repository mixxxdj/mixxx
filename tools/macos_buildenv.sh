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

if [ -n "${BUILDENV_ARM64}" ]; then
    VCPKG_TARGET_TRIPLET="arm64-osx-min1100-release"
    BUILDENV_BRANCH="2.5-rel"
    BUILDENV_NAME="mixxx-deps-2.5-$VCPKG_TARGET_TRIPLET-bea61fb"
    BUILDENV_SHA256="0c99cf740f16fd3a1b81ad17c5dcc94e9d18c6cdc92c3f6ca058fe0297a7cddd"
else
    if [ -n "${BUILDENV_RELEASE}" ]; then
        VCPKG_TARGET_TRIPLET="x64-osx-min1015-release"
        BUILDENV_BRANCH="2.5-rel"
        BUILDENV_NAME="mixxx-deps-2.5-$VCPKG_TARGET_TRIPLET-bea61fb"
        BUILDENV_SHA256="48c89e22e3c82b299d2e216a7a73c52a7b1c6961a2cce883df120ce610c09403"
    else
        VCPKG_TARGET_TRIPLET="x64-osx-min1015"
        BUILDENV_BRANCH="2.5"
        BUILDENV_NAME="mixxx-deps-2.5-$VCPKG_TARGET_TRIPLET-5d58718"
        BUILDENV_SHA256="c0b6d25c9bb5f7951014a06a03371e4190e0f8977805bf56c09dfa1ef2b285ae"
    fi
fi

MIXXX_ROOT="$(realpath "$(dirname "$THIS_SCRIPT_NAME")/..")"

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
                echo "Build environment $BUILDENV_NAME not found in mixxx repository, downloading https://downloads.mixxx.org/dependencies/${BUILDENV_BRANCH}/macOS/${BUILDENV_NAME}.zip"
                http_code=$(curl -sI -w "%{http_code}" "https://downloads.mixxx.org/dependencies/${BUILDENV_BRANCH}/macOS/${BUILDENV_NAME}.zip" -o /dev/null)
                if [ "$http_code" -ne 200 ]; then
                    echo "Downloading  failed with HTTP status code: $http_code"
                    exit 1
                fi
                curl "https://downloads.mixxx.org/dependencies/${BUILDENV_BRANCH}/macOS/${BUILDENV_NAME}.zip" -o "${BUILDENV_PATH}.zip"
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
                echo "Extracting ${BUILDENV_NAME}.zip..."
                unzip "${BUILDENV_PATH}.zip" -d "${BUILDENV_BASEPATH}" && \
                echo "Successfully extracted ${BUILDENV_NAME}.zip" && \
                rm "${BUILDENV_PATH}.zip"
            else
                echo "Build environment $BUILDENV_NAME not found in mixxx repository, run the command below to download it."
                echo "source ${THIS_SCRIPT_NAME} setup"
                return # exit would quit the shell being started
            fi
        elif [ "$1" != "--profile" ]; then
            echo "Build environment found: ${BUILDENV_PATH}"
        fi

        export MIXXX_VCPKG_ROOT="${BUILDENV_PATH}"
        export CMAKE_GENERATOR=Ninja
        export VCPKG_TARGET_TRIPLET="${VCPKG_TARGET_TRIPLET}"

        echo_exported_variables() {
            echo "MIXXX_VCPKG_ROOT=${MIXXX_VCPKG_ROOT}"
            echo "CMAKE_GENERATOR=${CMAKE_GENERATOR}"
            echo "VCPKG_TARGET_TRIPLET=${VCPKG_TARGET_TRIPLET}"
        }

        if [ -n "${GITHUB_ENV}" ]; then
            echo_exported_variables >> "${GITHUB_ENV}"
        elif [ "$1" != "--profile" ]; then
            echo ""
            echo "Exported environment variables:"
            echo_exported_variables
            echo "You can now configure cmake from the command line in an EMPTY build directory via:"
            echo "cmake -DCMAKE_TOOLCHAIN_FILE=${MIXXX_VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake ${MIXXX_ROOT}"
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
