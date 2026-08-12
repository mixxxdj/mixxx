#!/usr/bin/env bash
# Ignored in case of a source call, but needed for bash specific sourcing detection

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

HOST_ARCH=$(uname -m)  # One of x86_64, aarch64, etc.

# Buildenv defaults for the current Mixxx release.  Override any of these
# via environment variables when building a different version so the script
# does not need to be edited per release:
#   BUILDENV_BRANCH  — release series, also the download path segment ("2.7")
#   BUILDENV_NAME    — full archive basename
#                      ("mixxx-deps-2.7-x64-linux-1c20f84a")
#   VCPKG_TARGET_TRIPLET — vcpkg triplet for the host arch

case "$HOST_ARCH" in
    x86_64)
        if [ -n "${BUILDENV_RELEASE}" ]; then
            : "${VCPKG_TARGET_TRIPLET:=x64-linux-release}"
            : "${BUILDENV_BRANCH:=2.7-rel}"
            : "${BUILDENV_NAME:=mixxx-deps-2.7-x64-linux-rel-1c20f84a}"
        else
            : "${VCPKG_TARGET_TRIPLET:=x64-linux}"
            : "${BUILDENV_BRANCH:=2.7}"
            : "${BUILDENV_NAME:=mixxx-deps-2.7-x64-linux-1c20f84a}"
        fi
        ;;
    aarch64)
        VCPKG_TARGET_TRIPLET="arm64-linux"
        : "${BUILDENV_BRANCH:=2.7}"
        : "${BUILDENV_NAME:=mixxx-deps-2.7-arm64-linux}"
        echo "ERROR: arm64-linux buildenv is not yet published by Mixxx."
        echo "Once a mixxx-deps-<version>-arm64-linux-XXXXXXXX.zip appears on"
        echo "https://downloads.mixxx.org/dependencies/<version>/Linux/,"
        echo "set BUILDENV_NAME via the environment or in this script and re-run."
        exit 1
        ;;
    *)
        echo "ERROR: Unsupported architecture detected: $HOST_ARCH"
        echo "The AppImage buildenv is currently only available for x86_64 and aarch64."
        echo "Please refer to the following guide:"
        echo "https://github.com/mixxxdj/mixxx/wiki/Compiling-dependencies-for-Linux"
        return 1 2>/dev/null || exit 1
        ;;
esac

BUILDENV_URL="https://downloads.mixxx.org/dependencies/${BUILDENV_BRANCH}/Linux/${BUILDENV_NAME}.zip"
MIXXX_ROOT="$(realpath "$(dirname "$THIS_SCRIPT_NAME")/..")"

[ -z "$BUILDENV_BASEPATH" ] && BUILDENV_BASEPATH="${MIXXX_ROOT}/buildenv"

case "$1" in
    name)
        echo "$BUILDENV_NAME"
        if [ -n "${GITHUB_ENV}" ]; then
            echo "BUILDENV_NAME=$BUILDENV_NAME" >> "${GITHUB_ENV}"
        fi
        ;;

    setup)
        BUILDENV_PATH="${BUILDENV_BASEPATH}/${BUILDENV_NAME}"

        # vcpkg.cmake is at BUILDENV_PATH/scripts/buildsystems/, matching the
        # macos/android buildenv pattern.
        export MIXXX_VCPKG_ROOT="${BUILDENV_PATH}"

        export BUILDENV_NAME
        export BUILDENV_BASEPATH
        export MIXXX_VCPKG_ROOT
        export VCPKG_TARGET_TRIPLET="${VCPKG_TARGET_TRIPLET}"
        export CMAKE_PREFIX_PATH="${BUILDENV_PATH}/installed/${VCPKG_TARGET_TRIPLET}"

        # Install system dependencies needed for the AppImage build.
        if [ -n "${GITHUB_ENV}" ]; then
            sudo apt-get update
            sudo apt-get install -y --no-install-recommends \
                ccache \
                g++ \
                make \
                cmake \
                pkg-config \
                patchelf \
                file \
                desktop-file-utils \
                libfuse2t64 \
                unzip \
                squashfs-tools \
                libfaad-dev \
                libfdk-aac-dev \
                libglib2.0-dev \
                libsecret-1-dev \
                libgcrypt20-dev \
                libgpg-error-dev \
                libsqlite3-dev \
                libssl-dev \
                libprotobuf-dev \
                protobuf-compiler \
                libgl1-mesa-dev \
                libxkbcommon-dev \
                libxkbcommon-x11-dev \
                libxcb-glx0-dev \
                libxcb-cursor-dev \
                libxcb-keysyms1-dev \
                libxcb-randr0-dev \
                libxcb-xfixes0-dev \
                libxcb-xinerama0-dev \
                libxcb-xkb-dev \
                libxcb-shm0-dev \
                libxcb-icccm4-dev \
                libxcb-image0-dev \
                libxcb-util-dev \
                libxcb-sync-dev \
                libx11-xcb-dev \
                libxrender-dev \
                libxcb-xinput-dev \
                libupower-glib-dev \
                libsm-dev \
                libxrandr-dev \
                libxext-dev \
                libudev-dev
        fi

        echo_exported_variables() {
            echo "BUILDENV_NAME=${BUILDENV_NAME}"
            echo "BUILDENV_BASEPATH=${BUILDENV_BASEPATH}"
            echo "BUILDENV_URL=${BUILDENV_URL}"
            echo "MIXXX_VCPKG_ROOT=${MIXXX_VCPKG_ROOT}"
            echo "VCPKG_TARGET_TRIPLET=${VCPKG_TARGET_TRIPLET}"
            echo "CMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}"
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
        echo "Usage: source linux_appimage_buildenv.sh [options]"
        echo ""
        echo "options:"
        echo "   help       Displays this help."
        echo "   name       Displays the name of the required build environment."
        echo "   setup      Setup the build environment variables for download during CMake configuration."
        ;;
esac