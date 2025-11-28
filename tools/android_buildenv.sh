#!/usr/bin/env bash
# Ignored in case of a source call, but needed for bash specific sourcing detection

set -eo pipefail

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

if [ "$HOST_ARCH" == "x86_64" ]; then
    if [ -n "${BUILDENV_RELEASE}" ]; then
        echo "ERROR: No release VCPKG for Android yet!"
        exit 1
    else
        VCPKG_TARGET_TRIPLET="arm64-android"
        BUILDENV_BRANCH="2.7"
        BUILDENV_NAME="mixxx-deps-2.7-arm64-android-9dcfaf7"
        BUILDENV_SHA256="0821e7d4f6b989ed5acc3c9a8dafa00f479d9d8bfc8dea9f9b512816070c9bba"
    fi
else
    echo "ERROR: Unsupported architecture detected: $HOST_ARCH"
    echo "Please refer to the following guide to manually build the vcpkg environment:"
    echo "https://github.com/mixxxdj/mixxx/wiki/Compiling-dependencies-for-android"
    exit 1
fi

BUILDENV_URL="https://downloads.mixxx.org/dependencies/${BUILDENV_BRANCH}/Linux/${BUILDENV_NAME}.zip"
MIXXX_ROOT="$(realpath "$(dirname "$THIS_SCRIPT_NAME")/..")"
ANDROID_API=35
ANDROID_VERSION=35.0.0
ANDROID_NDK=27.2.12479018

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
        sudo apt-get update && sudo apt-get install -y --no-install-recommends -- \
            ccache \
            cmake \
            make \
            build-essential \
            '^libxcb.*-dev' \
            autoconf \
            autoconf-archive \
            bison \
            flex \
            google-android-cmdline-tools-13.0-installer \
            libasound2-dev \
            libegl1-mesa-dev \
            libghc-resolv-dev \
            libglu1-mesa-dev \
            libltdl-dev \
            libx11-xcb-dev \
            libxi-dev \
            libxkbcommon-dev \
            libxkbcommon-x11-dev \
            libxrender-dev \
            linux-libc-dev \
            openjdk-17-jdk \
            pkg-config \
            python3-jinja2
        (yes | sudo sdkmanager --licenses) || true
        sudo sdkmanager "platforms;android-${ANDROID_API}" "platform-tools" "build-tools;${ANDROID_VERSION}" "ndk;${ANDROID_NDK}"
        ANDROID_SDK=/usr/lib/android-sdk
        ANDROID_NDK_HOME=/usr/lib/android-sdk/ndk/${ANDROID_NDK}
        JAVA_HOME=$(find /usr/lib/jvm -maxdepth 1 -name 'java-17-openjdk*')
        export ANDROID_SDK
        export ANDROID_NDK_HOME
        export JAVA_HOME
        BUILDENV_PATH="${BUILDENV_BASEPATH}/${BUILDENV_NAME}"

        export BUILDENV_NAME
        export BUILDENV_BASEPATH
        export BUILDENV_URL
        export BUILDENV_SHA256
        export MIXXX_VCPKG_ROOT="${BUILDENV_PATH}"
        export VCPKG_TARGET_TRIPLET="${VCPKG_TARGET_TRIPLET}"

        echo_exported_variables() {
            echo "ANDROID_SDK=${ANDROID_SDK}"
            echo "ANDROID_NDK_HOME=${ANDROID_NDK_HOME}"
            echo "JAVA_HOME=${JAVA_HOME}"
            echo "BUILDENV_NAME=${BUILDENV_NAME}"
            echo "BUILDENV_BASEPATH=${BUILDENV_BASEPATH}"
            echo "BUILDENV_URL=${BUILDENV_URL}"
            echo "BUILDENV_SHA256=${BUILDENV_SHA256}"
            echo "MIXXX_VCPKG_ROOT=${MIXXX_VCPKG_ROOT}"
            echo "VCPKG_TARGET_TRIPLET=${VCPKG_TARGET_TRIPLET}"
        }

        if [ -n "${GITHUB_ENV}" ]; then
            echo_exported_variables >> "${GITHUB_ENV}"
        elif [ "$1" != "--profile" ]; then
            echo ""
            echo "Exported environment variables:"
            echo_exported_variables
            echo "You can now configure cmake from the command line in an EMPTY build directory via:"
            echo "cmake -DCMAKE_TOOLCHAIN_FILE=${MIXXX_VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake -DCMAKE_SYSTEM_NAME=Android ${MIXXX_ROOT}"
        fi
        ;;
    *)
        echo "Usage: source android_buildenv.sh [options]"
        echo ""
        echo "options:"
        echo "   help       Displays this help."
        echo "   name       Displays the name of the required build environment."
        echo "   setup      Setup the build environment variables for download during CMake configuration and install the build environment."
        ;;
esac
