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
    BUILDENV_BRANCH="2.6-rel"
    BUILDENV_NAME="mixxx-deps-2.6-arm64-osx-min1100-release-ccad946"
    BUILDENV_SHA256="1233c8265f433477b568a2ecdcc90fb26fd4c29da8860d475efbd591ae8f1d35"
else
    if [ -n "${BUILDENV_RELEASE}" ]; then
        VCPKG_TARGET_TRIPLET="x64-osx-min1100-release"
        BUILDENV_BRANCH="2.6-rel"
        BUILDENV_NAME="mixxx-deps-2.6-x64-osx-min1100-release-ccad946"
        BUILDENV_SHA256="bc50b8184ec3c155faeea190551645bbaedb05793c579b17a0f692b827da8c5f"
    else
        VCPKG_TARGET_TRIPLET="x64-osx-min1100"
        BUILDENV_BRANCH="2.6"
        BUILDENV_NAME="mixxx-deps-2.6-x64-osx-min1100-2dfaf1d"
        BUILDENV_SHA256="13f3c7e9657228dd6c65563dcdefe9d36a2688e5e2492ae5fa0ac3a545fc27ad"
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
