#!/bin/bash
# This script works with Debian, Ubuntu, and derivatives.
# shellcheck disable=SC1091
set -o pipefail

case "$1" in
    name)
        echo "No build environment name required for Debian based distros." >&2
        echo "This script installs the build dependencies via apt using the \"setup\" option." >&2
        ;;

    setup)
        source /etc/lsb-release 2>/dev/null
        case "${DISTRIB_CODENAME}" in
            bionic) # Ubuntu 18.04 LTS
                PACKAGES_EXTRA=(
                    libmp4v2-dev
                )
                ;;
            *) # libmp4v2 was removed from Debian 10 & Ubuntu 20.04 due to lack of maintenance, so use FFMPEG instead
                PACKAGES_EXTRA=(
                    libavformat-dev
                )
        esac

        sudo apt-get update

        # If jackd2 is installed as per dpkg database, install libjack-jackd2-dev.
        # This avoids a package deadlock, resulting in jackd2 being removed, and jackd1 being installed,
        # to satisfy portaudio19-dev's need for a jackd dev package. In short, portaudio19-dev needs a
        # jackd dev library, so let's give it one..
        if [ "$(dpkg-query -W -f='${Status}' jackd2 2>/dev/null | grep -c "ok installed")" -eq 1 ];
        then
            sudo apt-get install libjack-jackd2-dev;
        fi

        # Install a faster linker. Prefer mold, fall back to lld
        if apt-cache show mold 2>%1 >/dev/null;
        then
            sudo apt-get install -y --no-install-recommends mold
        else
            if apt-cache show lld 2>%1 >/dev/null;
            then
                sudo apt-get install -y --no-install-recommends lld
            fi
        fi

        sudo apt-get install -y --no-install-recommends -- \
            ccache \
            cmake \
            clazy \
            clang-tidy \
            debhelper \
            devscripts \
            docbook-to-man \
            dput \
            fonts-open-sans \
            fonts-ubuntu \
            g++ \
            lcov \
            libbenchmark-dev \
            libchromaprint-dev \
            libdistro-info-perl \
            libebur128-dev \
            libfaad-dev \
            libfftw3-dev \
            libflac-dev \
            libgmock-dev \
            libgtest-dev \
            libgl1-mesa-dev \
            libhidapi-dev \
            libid3tag0-dev \
            liblilv-dev \
            libmad0-dev \
            libmodplug-dev \
            libmp3lame-dev \
            libmsgsl-dev \
            libopus-dev \
            libopusfile-dev \
            libportmidi-dev \
            libprotobuf-dev \
            libqt6core5compat6-dev\
            libqt6opengl6-dev \
            libqt6sql6-sqlite \
            libqt6svg6-dev \
            librubberband-dev \
            libshout-idjc-dev \
            libsndfile1-dev \
            libsoundtouch-dev \
            libsqlite3-dev \
            libssl-dev \
            libtag1-dev \
            libudev-dev \
            libupower-glib-dev \
            libusb-1.0-0-dev \
            libwavpack-dev \
            lv2-dev \
            markdown \
            portaudio19-dev \
            protobuf-compiler \
            qtkeychain-qt6-dev \
            qt6-declarative-dev \
            qml-module-qtquick-controls \
            qml-module-qtquick-controls2 \
            qml-module-qt-labs-qmlmodels \
            qml-module-qtquick-shapes \
            qml6-module-* \
            "${PACKAGES_EXTRA[@]}"
        ;;
    *)
        echo "Usage: $0 [options]"
        echo ""
        echo "options:"
        echo "   help       Displays this help."
        echo "   name       Displays the name of the required build environment."
        echo "   setup      Installs the build environment."
        ;;
esac
