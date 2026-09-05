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
            : "${BUILDENV_NAME:=mixxx-deps-2.7-x64-linux-rel-8177263d}"
        else
            : "${VCPKG_TARGET_TRIPLET:=x64-linux}"
            : "${BUILDENV_BRANCH:=2.7}"
            : "${BUILDENV_NAME:=mixxx-deps-2.7-x64-linux-969b62ea}"
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
        exit 1
        ;;
esac

# Allow overriding the buildenv download URL (e.g. to point at a CI artifact
# while testing a not-yet-published buildenv).
: "${BUILDENV_URL:=https://downloads.mixxx.org/dependencies/${BUILDENV_BRANCH}/Linux/${BUILDENV_NAME}.zip}"
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
        export CMAKE_PREFIX_PATH="${BUILDENV_PATH}/installed/${VCPKG_TARGET_TRIPLET}"

        # System packages required for the build: build tools plus X11/Mesa/GL
        # headers and utilities (Qt platform). All other third-party libraries
        # come from the VCPKG buildenv. (The vcpkg toolchain prepends the
        # buildenv to CMAKE_PREFIX_PATH, so identical system packages are not used.)
        if command -v apt-get >/dev/null 2>&1; then
            sudo apt-get update
            # libfuse2t64 is the t64-transitioned name (Debian 13 / Ubuntu 24.04+);
            # libfuse2 is the older name (Ubuntu 22.04).  Install whichever is available.
            FUSE_PKG="libfuse2t64"
            if ! apt-cache show libfuse2t64 &>/dev/null; then
                FUSE_PKG="libfuse2"
            fi
            # XCB packages needed to link the static Qt plugin from the
            # buildenv; keep in sync with the buildenv's Qt build.
            sudo apt-get install -y --no-install-recommends \
                ccache \
                g++ \
                make \
                pkg-config \
                patchelf \
                file \
                desktop-file-utils \
                "${FUSE_PKG}" \
                unzip \
                squashfs-tools \
                libsecret-1-dev \
                libgcrypt20-dev \
                libgpg-error-dev \
                libgl1-mesa-dev \
                libx11-xcb-dev \
                libglu1-mesa-dev \
                libxrender-dev \
                libxi-dev \
                libxkbcommon-dev \
                libxkbcommon-x11-dev \
                libegl1-mesa-dev \
                libupower-glib-dev \
                libsm-dev \
                libxrandr-dev \
                libxext-dev \
                libudev-dev \
                libxcb1-dev \
                libxcb-cursor-dev \
                libxcb-glx0-dev \
                libxcb-icccm4-dev \
                libxcb-image0-dev \
                libxcb-keysyms1-dev \
                libxcb-randr0-dev \
                libxcb-render0-dev \
                libxcb-render-util0-dev \
                libxcb-shape0-dev \
                libxcb-shm0-dev \
                libxcb-sync-dev \
                libxcb-util-dev \
                libxcb-xfixes0-dev \
                libxcb-xkb-dev \
                libxcb-xinput-dev
        else
            echo "WARNING: The AppImage buildenv system-dependency step currently only"
            echo "automates Debian-based systems. Please install the equivalent"
            echo "packages for your distribution, or consider contributing a"
            echo "script for it."
        fi

        # The CPack AppImage generator requires CMake >= 4.2 (added in CMake
        # 4.2).  Priority: system CMake >= 4.2, then apt-get satisfy, then
        # fail with an install hint.  CI provides CMake via
        # jwlawson/actions-setup-cmake@v2.2 in build.yml, so this only
        # matters for local builds.
        if cmake --version 2>/dev/null | awk -F'[ .]' 'NR==1 {ok=($3>4 || ($3==4 && $4>=2)); exit !ok} END {if (NR==0) exit 1}'; then
            echo "Using system CMake (>= 4.2)"
        elif command -v apt-get >/dev/null 2>&1 && sudo apt-get satisfy "cmake (>= 4.2)"; then
            echo "CMake >= 4.2 installed via apt"
        else
            echo "CMake >= 4.2 is required for the AppImage CPack generator, but no"
            echo "version >= 4.2 is available in the system package manager."
            echo ""
            if command -v snap >/dev/null 2>&1; then
                echo "On Ubuntu, install it via snap:"
                echo "  sudo snap install cmake --channel=4.2/stable --classic"
            else
                echo "Please install CMake >= 4.2 (e.g. from https://cmake.org/download/)"
            fi
            echo "and re-source this script."
            return 1
        fi

        # appimagetool is required by the CPack AppImage generator, which searches
        # for an executable named "appimagetool" in the PATH (or via
        # CPACK_APPIMAGE_TOOL_EXECUTABLE).  Install it to /usr/local/bin
        # which is in the default PATH.  Running it requires FUSE
        # (libfuse2t64 is installed above where apt-get is available).
        APPIMAGETOOL_URL="https://github.com/AppImage/appimagetool/releases/download/continuous/appimagetool-${HOST_ARCH}.AppImage"
        sudo curl -fsSL --connect-timeout 15 --max-time 120 \
            -o /usr/local/bin/appimagetool \
            "${APPIMAGETOOL_URL}"
        sudo chmod +x /usr/local/bin/appimagetool

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
        echo "Usage: source appimage_buildenv.sh [options]"
        echo ""
        echo "options:"
        echo "   help       Displays this help."
        echo "   name       Displays the name of the required build environment."
        echo "   setup      Setup the build environment variables for download during CMake configuration."
        ;;
esac
