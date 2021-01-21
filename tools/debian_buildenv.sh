#!/bin/bash
# This script works with Debian, Ubuntu, and derivatives.
# shellcheck disable=SC1091
set -o pipefail

COMMAND=$1
shift

case "$COMMAND" in
    name)
        echo "No build environment needed for Ubuntu, please install dependencies using apt." >&2
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
                    libavcodec-dev
                    libavutil-dev
                )
        esac

        sudo apt-get update
        sudo apt-get install -y --no-install-recommends -- \
            ccache \
            cmake \
            debhelper \
            devscripts \
            docbook-to-man \
            dput \
            fonts-open-sans \
            fonts-ubuntu \
            g++ \
            libavformat-dev \
            libchromaprint-dev \
            libdistro-info-perl \
            libebur128-dev \
            libfaad-dev \
            libfftw3-dev \
            libflac-dev \
            libhidapi-dev \
            libid3tag0-dev \
            liblilv-dev \
            libmad0-dev \
            libmodplug-dev \
            libmp3lame-dev \
            libopus-dev \
            libopusfile-dev \
            libportmidi-dev \
            libprotobuf-dev \
            libqt5opengl5-dev \
            libqt5sql5-sqlite \
            libqt5svg5-dev \
            libqt5x11extras5-dev \
            librubberband-dev \
            libshout3-dev \
            libsndfile1-dev \
            libsoundtouch-dev \
            libsqlite3-dev \
            libtag1-dev \
            libupower-glib-dev \
            libusb-1.0-0-dev \
            libwavpack-dev \
            markdown \
            portaudio19-dev \
            protobuf-compiler \
            qt5keychain-dev \
            qtdeclarative5-dev \
            "${PACKAGES_EXTRA[@]}"
esac
