#!/bin/bash
set -o pipefail

if [ -z "${GITHUB_ENV}" ] && ! $(return 0 2>/dev/null); then
  echo "This script must be run by sourcing it:"
  echo "source $0 $@"
  exit 1
fi

COMMAND=$1
shift 1

realpath() {
    OLDPWD="${PWD}"
    cd "$1" || exit 1
    pwd
    cd "${OLDPWD}" || exit 1
}

# some hackery is required to be compatible with both bash and zsh
THIS_SCRIPT_NAME=${BASH_SOURCE[0]}
[ -z "$THIS_SCRIPT_NAME" ] && THIS_SCRIPT_NAME=$0

MIXXX_ROOT="$(realpath "$(dirname "$THIS_SCRIPT_NAME")/..")"

read -d'\n' BUILDENV_NAME BUILDENV_SHA256 < "${MIXXX_ROOT}/packaging/macos/build_environment"

[ -z "$BUILDENV_BASEPATH" ] && BUILDENV_BASEPATH="${MIXXX_ROOT}/buildenv"

case "$COMMAND" in
    name)
        if [ -n "${GITHUB_ENV}" ]; then
            echo "BUILDENV_NAME=$envname" >> "${GITHUB_ENV}"
        else
            echo "$BUILDENV_NAME"
        fi
        ;;

    setup)
        if [[ "$BUILDENV_NAME" =~ .*macosminimum([0-9]*\.[0-9]*).* ]]; then
            # bash and zsh have different ways of getting the matched string
            # zsh's BASH_REMATCH option is not actually compatible with bash
            if [ -n "${BASH_REMATCH}" ]; then
                export MACOSX_DEPLOYMENT_TARGET="${BASH_REMATCH[1]}"
            elif [ -n "$match" ]; then
                export MACOSX_DEPLOYMENT_TARGET="${match[1]}"
            fi
        else
            echo "Build environment did not match expected pattern. Check ${MIXXX_ROOT}/packaging/macos/build_environment file." >&2
            return
        fi

        BUILDENV_PATH="${BUILDENV_BASEPATH}/${BUILDENV_NAME}"
        mkdir -p "${BUILDENV_BASEPATH}"
        if [ ! -d "${BUILDENV_PATH}" ]; then
            if [ "$1" != "--profile" ]; then
                echo "Build environment $BUILDENV_NAME not found in mixxx repository, downloading it..."
                curl "https://downloads.mixxx.org/builds/buildserver/2.3.x-unix/${BUILDENV_NAME}.tar.gz" -o "${BUILDENV_PATH}.tar.gz"
                OBSERVED_SHA256=$(shasum -a 256 "${BUILDENV_PATH}.tar.gz"|cut -f 1 -d' ')
                if [[ $OBSERVED_SHA256 == $BUILDENV_SHA256 ]]; then
                    echo "Download matched expected SHA256 sum $BUILDENV_SHA256"
                else
                    echo "ERROR: Download did not match expected SHA256 checksum!"
                    echo "Expected $BUILDENV_SHA256"
                    echo "Got $OBSERVED_SHA256"
                    exit 1
                fi
                echo ""
                echo "Extracting ${BUILDENV_NAME}.tar.gz..."
                tar xf "${BUILDENV_PATH}.tar.gz" -C "${BUILDENV_BASEPATH}" && \
                echo "Successfully extracted ${BUILDENV_NAME}.tar.gz"
            else
                echo "Build environment $BUILDENV_NAME not found in mixxx repository, run the command below to download it."
                echo "source ${THIS_SCRIPT_NAME} setup"
                return # exit would quit the shell being started
            fi
        elif [ "$1" != "--profile" ]; then
            echo "Build environment found: ${BUILDENV_PATH}"
        fi

        export SDKROOT="${BUILDENV_BASEPATH}/MacOSX10.13.sdk"
        if [ -d "${SDKROOT}" ]; then
            if [ "$1" != "--profile" ]; then
                echo "macOS 10.13 SDK found: ${SDKROOT}"
            fi
        else
            echo "macOS 10.13 SDK not found, downloading it..."
            curl -L "https://github.com/phracker/MacOSX-SDKs/releases/download/10.15/MacOSX10.13.sdk.tar.xz" -o "${SDKROOT}.tar.xz"
            OBSERVED_SHA256=$(shasum -a 256 "${SDKROOT}.tar.xz"|cut -f 1 -d' ')
            EXPECTED_SHA256="a3a077385205039a7c6f9e2c98ecdf2a720b2a819da715e03e0630c75782c1e4"
            if [[ $OBSERVED_SHA256 == $EXPECTED_SHA256 ]]; then
                echo "Download matched expected SHA256 sum $EXPECTED_SHA256"
            else
                echo "ERROR: Download did not match expected SHA256 checksum!"
                echo "Expected $EXPECTED_SHA256"
                echo "Got $OBSERVED_SHA256"
                exit 1
            fi
            echo "Extracting MacOSX10.13.sdk.tar.xz..."
            tar xf "${SDKROOT}.tar.xz" -C "${BUILDENV_BASEPATH}" && \
            echo "Successfully extacted MacOSX10.13.sdk.tar.xz"
            rm "${SDKROOT}.tar.xz"
        fi

        export CC="/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang"
        export CXX="/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang++"
        export CMAKE_PREFIX_PATH="${BUILDENV_PATH}"
        export Qt5_DIR="$(find "${BUILDENV_PATH}" -type d -path "*/cmake/Qt5")"
        export QT_QPA_PLATFORM_PLUGIN_PATH="$(find "${BUILDENV_PATH}" -type d -path "*/plugins")"
        export PATH="${BUILDENV_PATH}/bin:${PATH}"

        echo_exported_variables() {
            echo "CC=${CC}"
            echo "CXX=${CXX}"
            echo "SDKROOT=${SDKROOT}"
            echo "MACOSX_DEPLOYMENT_TARGET=${MACOSX_DEPLOYMENT_TARGET}"
            echo "CMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}"
            echo "Qt5_DIR=${Qt5_DIR}"
            echo "QT_QPA_PLATFORM_PLUGIN_PATH=${QT_QPA_PLATFORM_PLUGIN_PATH}"
            echo "PATH=${PATH}"
        }

        if [ -n "${GITHUB_ENV}" ]; then
            echo_exported_variables >> "${GITHUB_ENV}"
        elif [ "$1" != "--profile" ]; then
            echo ""
            echo "Exported environment variables:"
            echo_exported_variables
        fi
        ;;
esac
