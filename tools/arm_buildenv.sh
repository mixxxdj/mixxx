#!/bin/bash
set -o pipefail

DISTRIB=$1
shift

case "$DISTRIB" in
    buster)
        PACKAGES_EXTRA=""
        ;;
    ubuntu20.04)
        PACKAGES_EXTRA="libavutil-dev"
        ;;
    *)
        echo "Please specify a distribution" >&2
        ;;
esac

echo $PACKAGES_EXTRA

apt-get update -y -q
apt-get install -y --no-install-recommends \
    ccache \
    libgl1-mesa-dri libgles1 \
    libavformat-dev \
    libavcodec-dev \
    libchromaprint-dev \
    libebur128-dev \
    libfftw3-dev \
    libflac-dev \
    libfaad-dev \
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
    portaudio19-dev \
    protobuf-compiler \
    qt5-default \
    qt5keychain-dev \
    qtscript5-dev \
    ${PACKAGES_EXTRA}
