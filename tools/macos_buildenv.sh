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

HOST_ARCH=$(uname -m)  # One of x86_64, arm64, i386, ppc or ppc64

if [ "$HOST_ARCH" == "x86_64" ]; then
	if [ -n "${BUILDENV_ARM64_CROSS}" ]; then
	    if [ -n "${BUILDENV_RELEASE}" ]; then
	        VCPKG_TARGET_TRIPLET="arm64-osx-min1100-release"
	        BUILDENV_BRANCH="2.6-rel"
	        BUILDENV_NAME="mixxx-deps-2.6-arm64-osx-cross-rel-da4c207"
	        BUILDENV_SHA256="4a6e7a2d83ae2056a89298a0fd6d110d44a4344e3968219dd0c3d34648599233"
	    else
	        VCPKG_TARGET_TRIPLET="arm64-osx-min1100"
	        BUILDENV_BRANCH="2.6"
	        BUILDENV_NAME="mixxx-deps-2.6-arm64-osx-cross-c2def9b"
	        BUILDENV_SHA256="fa38900a64dec28c43888ec6f36421e9a8dc31e37115831fc779beca071014d7"
	    fi
	else
	    if [ -n "${BUILDENV_RELEASE}" ]; then
	        VCPKG_TARGET_TRIPLET="x64-osx-min1100-release"
	        BUILDENV_BRANCH="2.6-rel"
	        BUILDENV_NAME="mixxx-deps-2.6-x64-osx-rel-da4c207"
	        BUILDENV_SHA256="5e7f9a4d5d58a3b720a64d179f9cf72dab109460e74e9e31e9c2ada0885bb4a0"
	    else
	        VCPKG_TARGET_TRIPLET="x64-osx-min1100"
	        BUILDENV_BRANCH="2.6"
	        BUILDENV_NAME="mixxx-deps-2.6-x64-osx-c2def9b"
	        BUILDENV_SHA256="0c75b39d6c03e34e794ab95cc460b1d11a0b976d572e31451b7c0798d9035d73"
	    fi
	fi
elif [ "$HOST_ARCH" == "arm64" ]; then
    if [ -n "${BUILDENV_RELEASE}" ]; then
        VCPKG_TARGET_TRIPLET="arm64-osx-min1100-release"
        BUILDENV_BRANCH="2.6-rel"
        BUILDENV_NAME="mixxx-deps-2.6-arm64-osx-rel-da4c207"
        BUILDENV_SHA256="5e47b9f3dcca509be324779ce83aa342c29680a8924f83b2ecac3021b7ad3e87"
    else
        VCPKG_TARGET_TRIPLET="arm64-osx-min1100"
        BUILDENV_BRANCH="2.6"
        BUILDENV_NAME="mixxx-deps-2.6-arm64-osx-c2def9b"
        BUILDENV_SHA256="c6a00220d9b938dedec4864dbcb4c2db2d1ca9e6f8f60c20883c0008a163db6f"
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
