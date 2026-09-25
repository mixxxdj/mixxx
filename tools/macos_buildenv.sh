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
	        BUILDENV_BRANCH="2.7-rel"
	        BUILDENV_NAME="mixxx-deps-2.7-arm64-osx-cross-rel-ee6dc7fe"
	        BUILDENV_SHA256="3e7ac736b13d92c3801c1e9bc2ecea6602b1663acd13f68c0e1a162939a142fb"
	    else
	        VCPKG_TARGET_TRIPLET="arm64-osx-min1100"
	        BUILDENV_BRANCH="2.7"
	        BUILDENV_NAME="mixxx-deps-2.7-arm64-osx-cross-1c20f84a"
	        BUILDENV_SHA256="d0c1df3b8c5414ee1d7e444ebbdf1215e3e3e1585b9255c607720af2438b7b96"
	    fi
	else
	    if [ -n "${BUILDENV_RELEASE}" ]; then
	        VCPKG_TARGET_TRIPLET="x64-osx-min1100-release"
	        BUILDENV_BRANCH="2.7-rel"
	        BUILDENV_NAME="mixxx-deps-2.7-x64-osx-rel-ee6dc7fe"
	        BUILDENV_SHA256="0a7481cc3e7b2ec2eaff7b48804a1eda7506c01a0b0f326ffb3d12bd3787b950"
	    else
	        VCPKG_TARGET_TRIPLET="x64-osx-min1100"
	        BUILDENV_BRANCH="2.7"
	        BUILDENV_NAME="mixxx-deps-2.7-x64-osx-1c20f84a"
	        BUILDENV_SHA256="617c9cca081520c63fcbb7dc9bc0a8f44044ddbe63af7a53016e344d979e82cd"
	    fi
	fi
elif [ "$HOST_ARCH" = "arm64" ]; then
    if [ -n "${BUILDENV_RELEASE}" ]; then
        VCPKG_TARGET_TRIPLET="arm64-osx-min1100-release"
        BUILDENV_BRANCH="2.7-rel"
        BUILDENV_NAME="mixxx-deps-2.7-arm64-osx-rel-ee6dc7fe"
        BUILDENV_SHA256="4dec26e22803decbf0ce664ef53c12e6d846df7222bb48d3a47999878e9ba02c"
    else
        VCPKG_TARGET_TRIPLET="arm64-osx-min1100"
        BUILDENV_BRANCH="2.7"
        BUILDENV_NAME="mixxx-deps-2.7-arm64-osx-1c20f84a"
        BUILDENV_SHA256="3771ecabe4284225a358e38ca9720a7c8b2a78521cf4b20a443dae19d9d54b37"
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
