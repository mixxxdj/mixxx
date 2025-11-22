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
            BUILDENV_NAME="mixxx-deps-2.6-arm64-osx-cross-rel-37fb293"
            BUILDENV_SHA256="9399e3719e35a344c14e2a31b30029b5e8be4db078239a93256d06dc186e7808"
	    else
	        VCPKG_TARGET_TRIPLET="arm64-osx-min1100"
	        BUILDENV_BRANCH="2.6"
	        BUILDENV_NAME="mixxx-deps-2.6-arm64-osx-cross-2de72ad"
	        BUILDENV_SHA256="46a3e6be2b034d20a0cd2cae168e5741d93205b449e13042ff9ca43ed3cedf9a"
	    fi
	else
	    if [ -n "${BUILDENV_RELEASE}" ]; then
	        VCPKG_TARGET_TRIPLET="x64-osx-min1100-release"
	        BUILDENV_BRANCH="2.6-rel"
            BUILDENV_NAME="mixxx-deps-2.6-x64-osx-rel-37fb293"
            BUILDENV_SHA256="accf4b5171aa7ee40d2d9f98ad4bd3315487c14ca7c22c2705adaf1944f5e688"
	    else
	        VCPKG_TARGET_TRIPLET="x64-osx-min1100"
	        BUILDENV_BRANCH="2.6"
            BUILDENV_NAME="mixxx-deps-2.6-x64-osx-2de72ad"
            BUILDENV_SHA256="8a26bf23dd9e5d732e5b755cce30f30bf00ff3408e1d45d58a00affd892debde"
	    fi
	fi
elif [ "$HOST_ARCH" = "arm64" ]; then
    if [ -n "${BUILDENV_RELEASE}" ]; then
        VCPKG_TARGET_TRIPLET="arm64-osx-min1100-release"
        BUILDENV_BRANCH="2.6-rel"
        BUILDENV_NAME="mixxx-deps-2.6-arm64-osx-rel-37fb293"
        BUILDENV_SHA256=""
    else
        VCPKG_TARGET_TRIPLET="arm64-osx-min1100"
        BUILDENV_BRANCH="2.6"
        BUILDENV_NAME="mixxx-deps-2.6-arm64-osx-2de72ad"
        BUILDENV_SHA256="1a6f2076da506f9fccd7409cb432b96606ac16eba10b2046e1bbfdb1f7dee456"
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
