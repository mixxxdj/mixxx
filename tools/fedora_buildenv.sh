#!/bin/bash
# This script works with Fedora Worstation 39 (and possibly other RPM based distros, not tested)
# shellcheck disable=SC1091
set -o pipefail

case "$1" in
    name)
        echo "No build environment name required for Debian based distros." >&2
        echo "This script installs the build dependencies via apt using the \"setup\" option." >&2
        ;;

    setup)
        source /etc/os-release 2>/dev/null
        case "${ID}" in
            fedora) # Fedora 39
                echo "Tested with Fedora 39 for mixxx 2.4 branch\n"
                ;;
            *) # untested
                echo "Untested distro\n"
                                
        esac

        echo "Enabling RPMFusion free and nonfree repos"
		sudo dnf install -y \
			https://mirrors.rpmfusion.org/free/fedora/rpmfusion-free-release-$(rpm -E %fedora).noarch.rpm\
			https://mirrors.rpmfusion.org/nonfree/fedora/rpmfusion-nonfree-release-$(rpm -E %fedora).noarch.rpm

		sudo dnf config-manager --enable fedora-cisco-openh264

		sudo dnf groupinstall -y "C Development Tools and Libraries"

		# there is some conflict with ffmpeg that needs checking
        echo "Special case for ffmpeg-devel, may remove other packages"
		sudo dnf install -y \
			ffmpeg-devel --allowerasing

        echo "Installing remaining dependencies (from .spec file)"
		sudo dnf install -y\
			chrpath\
			cmake\
			faad2\
			fftw-devel\
			flac-devel\
			gmock-devel\
			google-benchmark-devel\
			gtest-devel\
			guidelines-support-library-devel\
			hidapi-devel\
			lame-devel\
			libchromaprint-devel\
			libebur128-devel\
			libGL-devel\
			libGLU-devel\
			libid3tag-devel\
			libmad-devel\
			libmodplug-devel\
			libmp4v2-devel\
			libsndfile-devel\
			libusbx-devel\
			libvorbis-devel\
			lilv-devel\
			ninja-build\
			opus-devel\
			opusfile-devel\
			portaudio-devel\
			portmidi-devel\
			protobuf-compiler\
			protobuf-devel\
			protobuf-lite-devel\
			qt5-qtbase-devel\
			qt5-qtdeclarative-devel\
			qt5-qtmultimedia-devel\
			qt5-qtscript-devel\
			qt5-qtsvg-devel\
			qt5-qtx11extras-devel\
			qt6-qt5compat-devel\
			qt6-qtbase-devel\
			qt6-qtmultimedia-devel\
			qt6-qtsvg-devel\
			qtkeychain-qt5-devel\
			qtkeychain-qt6-devel\
			rubberband-devel\
			soundtouch-devel\
			sqlite-devel\
			taglib-devel\
			upower-devel\
			wavpack-devel\
			zlib-devel

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
