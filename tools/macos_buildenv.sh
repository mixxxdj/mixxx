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
    if [ -n "${BUILDENV_RELEASE}" ]; then
        BUILDENV_BRANCH="2.4-rel"
        BUILDENV_NAME="mixxx-deps-rel-2.4-arm64-osx-min1100-5adfbef"
        BUILDENV_SHA256="17c9e5d552b160fe0e3de605ee6533f27f9ce43bd7d443f3c0fc315bed1a81a2"
    else
        BUILDENV_BRANCH="2.4"
        BUILDENV_NAME="mixxx-deps-2.4-arm64-osx-min1100-65126fd"
        BUILDENV_SHA256="1274cd167eb8966e86b86f8f65c6fd7d28f6b3b0ec7f9fd6527ece4588463749"
    fi
else
    if [ -n "${BUILDENV_RELEASE}" ]; then
        BUILDENV_BRANCH="2.4-rel"
        BUILDENV_NAME="mixxx-deps-rel-2.4-x64-osx-min1012-5adfbef"
        BUILDENV_SHA256="5b2fc067b37cc206090531cb7dfe6f9e0be699de0183629f0838706acd73171e"
    else
        BUILDENV_BRANCH="2.4"
        BUILDENV_NAME="mixxx-deps-2.4-x64-osx-min1012-65126fd"
        BUILDENV_SHA256="c20872e2f73977db7d7b4b7199eed449fe7710da891076c2e8a079ea3d38d5ce"
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
                echo "Build environment $BUILDENV_NAME not found in mixxx repository, downloading it..."
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

        echo_exported_variables() {
            echo "MIXXX_VCPKG_ROOT=${MIXXX_VCPKG_ROOT}"
            echo "CMAKE_GENERATOR=${CMAKE_GENERATOR}"
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
