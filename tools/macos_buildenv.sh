#!/usr/bin/env bash
# Ignored in case of a source call, but needed for bash specific sourcing detection

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

# Get script file location, compatible with bash and zsh
if [ -n "$BASH_VERSION" ]; then
  THIS_SCRIPT_NAME="${BASH_SOURCE[0]}"
elif [ -n "$ZSH_VERSION" ]; then
  # shellcheck disable=SC2296
  THIS_SCRIPT_NAME="${(%):-%N}"
else
  THIS_SCRIPT_NAME="$0"
fi

HOST_ARCH=$(uname -m)  # One of x86_64, arm64, i386, ppc or ppc64

if [ "$HOST_ARCH" = "x86_64" ]; then
	if [ -n "${BUILDENV_ARM64_CROSS}" ]; then
	    if [ -n "${BUILDENV_RELEASE}" ]; then
	        VCPKG_TARGET_TRIPLET="arm64-osx-min1100-release"
	        BUILDENV_BRANCH="2.6-rel"
	        BUILDENV_NAME="mixxx-deps-2.6-arm64-osx-cross-rel-f46c645"
	        BUILDENV_SHA256="191fc876dfaaa49854d5a113b4bfe8e35bfa7d7b58118b645f0d2f2830839e51"
	    else
	        VCPKG_TARGET_TRIPLET="arm64-osx-min1100"
	        BUILDENV_BRANCH="2.6"
	        BUILDENV_NAME="mixxx-deps-2.6-arm64-osx-cross-aa78b5a"
	        BUILDENV_SHA256="75187939a0847c98dd9c0c64a0bb35f7e8878f2288ac2e22ab99635f598bdd9a"
	    fi
	else
	    if [ -n "${BUILDENV_RELEASE}" ]; then
	        VCPKG_TARGET_TRIPLET="x64-osx-min1100-release"
	        BUILDENV_BRANCH="2.6-rel"
	        BUILDENV_NAME="mixxx-deps-2.6-x64-osx-rel-f46c645"
	        BUILDENV_SHA256="c28e64727743c126ea56c152a328a442e5bf8bb20705a2b232c1677ed290e0bb"
	    else
	        VCPKG_TARGET_TRIPLET="x64-osx-min1100"
	        BUILDENV_BRANCH="2.6"
	        BUILDENV_NAME="mixxx-deps-2.6-x64-osx-aa78b5a"
	        BUILDENV_SHA256="ccb81981c4a45a65453531ac6a5d0d0ea18e0e1383b9c4fd09077125efce8391"
	    fi
	fi
elif [ "$HOST_ARCH" = "arm64" ]; then
    if [ -n "${BUILDENV_RELEASE}" ]; then
        VCPKG_TARGET_TRIPLET="arm64-osx-min1100-release"
        BUILDENV_BRANCH="2.6-rel"
        BUILDENV_NAME="mixxx-deps-2.6-arm64-osx-rel-f46c645"
        BUILDENV_SHA256="fd7873545ef564397f505c7f2611a0ab59091636cece7438cd10bf054fb7c950"
    else
        VCPKG_TARGET_TRIPLET="arm64-osx-min1100"
        BUILDENV_BRANCH="2.6"
        BUILDENV_NAME="mixxx-deps-2.6-arm64-osx-aa78b5a"
        BUILDENV_SHA256="560b1572d65480a1773f70c320745f3fab8bb4a12380764d31d9547b38069401"
    fi
else
    echo "ERROR: Unsupported architecture detected: $HOST_ARCH"
    echo "Please refer to the following guide to manually build the vcpkg environment:"
    echo "https://github.com/mixxxdj/mixxx/wiki/Compiling-dependencies-for-macOS-arm64"
    exit 1
fi

BUILDENV_URL="https://downloads.mixxx.org/dependencies/${BUILDENV_BRANCH}/macOS/${BUILDENV_NAME}.zip"
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

        export BUILDENV_NAME
        export BUILDENV_BASEPATH
        export BUILDENV_URL
        export BUILDENV_SHA256
        export MIXXX_VCPKG_ROOT="${BUILDENV_PATH}"
        export CMAKE_GENERATOR=Ninja
        export VCPKG_TARGET_TRIPLET="${VCPKG_TARGET_TRIPLET}"

        echo_exported_variables() {
            echo "BUILDENV_NAME=${BUILDENV_NAME}"
            echo "BUILDENV_BASEPATH=${BUILDENV_BASEPATH}"
            echo "BUILDENV_URL=${BUILDENV_URL}"
            echo "BUILDENV_SHA256=${BUILDENV_SHA256}"
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
        echo "   setup      Setup the build environment variables for download during CMake configuration."
        ;;
esac
