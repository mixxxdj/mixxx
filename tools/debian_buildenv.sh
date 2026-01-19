#!/bin/bash
# This script works with Debian, Ubuntu, and derivatives.
# shellcheck disable=SC1091

# Fail if not executed with bash
if [ -z "$BASH_VERSION" ]; then
    echo "Error: This script must be called as executable: ./debian_buildenv.sh setup" >&2
    exit 1
fi

set -o pipefail

case "$1" in
    name)
        echo "No build environment name required for Debian based distros." >&2
        echo "This script installs the build dependencies via apt using the \"setup\" option." >&2
        ;;

    setup)
        source /etc/os-release 2>/dev/null
        case "${VERSION_CODENAME}" in
            jammy|bullseye|victoria|vera|vanessa|virginia) # <= Ubuntu 22.04.5 LTS
                PACKAGES_EXTRA=(
                    libqt6shadertools6-dev
                )
                ;;
            *)
                PACKAGES_EXTRA=(
                    qt6-shadertools-dev
                )
        esac

        case "${VERSION_CODENAME}" in
            jammy|noble|oracular|bullseye|bookworm|victoria|vera|vanessa|virginia|wilma|wildflower) # <= Ubuntu 24.10
                # qt6-svg-plugins not available
                ;;
            *)
                PACKAGES_EXTRA+=(
                    qt6-svg-plugins
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

        # Ask user if they want to install stemgen support
        echo ""
        echo "Do you want to install axeldelafosse stemgen for stem conversion support? (y/n)"
        read -p "Install stemgen? " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            INSTALL_STEMGEN=true

            # Add stemgen dependencies if user wants it
            PACKAGES+=(
                ffmpeg
                sox
                pipx
                python3-full
            )
        else
            INSTALL_STEMGEN=false
        fi

        # Install a faster linker. Prefer mold, fall back to lld
        if apt-cache show mold 2>/dev/null >/dev/null;
        then
            sudo apt-get install -y --no-install-recommends mold
        else
            if apt-cache show lld 2>/dev/null >/dev/null;
            then
                sudo apt-get install -y --no-install-recommends lld
            fi
        fi

	      # Check if fonts-ubuntu is available (from non-free repository)
        if ! apt-cache show fonts-ubuntu 2>/dev/null | grep -q "Package: fonts-ubuntu"; then
            echo ""
            echo "⚠️  WARNING: The package 'fonts-ubuntu' is not available."
            echo "This package is required for Mixxx and is located in the Debian non-free repository."
            echo ""
            read -p "Do you want to enable the non-free repository and install fonts-ubuntu? (y/n) " -n 1 -r
            echo
            if [[ $REPLY =~ ^[Yy]$ ]]; then
                echo "Enabling non-free repository..."
                # Add non-free to sources.list if not already present
                if ! grep -q " non-free$" /etc/apt/sources.list; then
                    sudo sed -i 's/^\(deb.*\) \(main\|contrib\|non-free-firmware\)$/\1 \2 non-free/' /etc/apt/sources.list
                fi
                echo "Updating package list..."
                sudo apt-get update
                FONTS_UBUNTU_AVAILABLE=true
            else
                echo "Continuing without fonts-ubuntu..."
                FONTS_UBUNTU_AVAILABLE=false
            fi
        else
            FONTS_UBUNTU_AVAILABLE=true
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
            $([ "$FONTS_UBUNTU_AVAILABLE" = true ] && echo "fonts-ubuntu") \
            g++ \
            lcov \
            libavformat-dev \
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
            libqt6core5compat6-dev \
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
            qt6-declarative-private-dev \
            qt6-base-private-dev \
            qt6-qpa-plugins \
            qml6-module-qt5compat-graphicaleffects \
            qml6-module-qtqml-workerscript \
            qml6-module-qtquick-controls \
            qml6-module-qtquick-layouts \
            qml6-module-qtquick-shapes \
            qml6-module-qtquick-templates \
            qml6-module-qtquick-window \
            qml6-module-qt-labs-qmlmodels \
            "${PACKAGES_EXTRA[@]}"

        # Install stemgen if user requested it
        if [ "$INSTALL_STEMGEN" = true ]; then
            echo ""
            echo "Installing stemgen for the user running sudo..."

            # Get the actual user running sudo
            ACTUAL_USER="${SUDO_USER:-$USER}"

            # Install stemgen as the actual user
            sudo -u "$ACTUAL_USER" pipx install stemgen

            if [ $? -eq 0 ]; then
                echo "✓ stemgen installed successfully to $ACTUAL_USER"
                echo "Location: /home/$ACTUAL_USER/.local/bin/stemgen"
            else
                echo "⚠️  Failed to install stemgen"
            fi
        fi

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
