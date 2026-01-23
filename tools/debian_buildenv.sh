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

        # Ask user if he/she wants to install demucs support for stem conversion
        echo ""
        echo "Do you want to install stem conversion support? (y/n)"
        read -p "Install stem conversion support? " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            INSTALL_DEMUCS=true

            # Add demucs dependencies if user wants it
            PACKAGES+=(
                ffmpeg
                sox
                python3-full
                python3-venv
            )
        else
            INSTALL_DEMUCS=false
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
            echo "WARNING: The package 'fonts-ubuntu' is not available."
            echo "This package is required for Mixxx and is located in the Debian non-free repository."
            echo ""
            echo "See also: https://wiki.debian.org/SourcesList"
            echo ""
            read -p "Would you like to exit to enable 'non-free' now? (y = Exit / n = Continue without fonts): " -n 1 -r
            echo
            if [[ $REPLY =~ ^[Yy]$ ]]; then
                echo "Please edit your /etc/apt/sources.list, run 'sudo apt update', and restart this script."
                exit 1
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

        # Install demucs if user requested it
        if [ "$INSTALL_DEMUCS" = true ]; then
            # Get the actual user running sudo
            ACTUAL_USER="${SUDO_USER:-$USER}"

            echo ""
            echo "Setting up Python virtual environment for Mixxx stem conversion..."
            echo ""

            # Create virtual environment
            VENV_PATH="/home/$ACTUAL_USER/.local/mixxx_venv"

            if [ -d "$VENV_PATH" ]; then
                echo "✓ Virtual environment already exists at $VENV_PATH"
            else
                echo "Creating virtual environment at $VENV_PATH..."
                sudo -u "$ACTUAL_USER" python3 -m venv "$VENV_PATH"

                if [ $? -eq 0 ]; then
                    echo "✓ Virtual environment created successfully"
                else
                    echo "⚠️  Failed to create virtual environment"
                    exit 1
                fi
            fi

            # Upgrade pip and setuptools in the virtual environment
            echo ""
            echo "Upgrading pip and setuptools..."
            sudo -u "$ACTUAL_USER" "$VENV_PATH/bin/pip" install --upgrade pip setuptools wheel

            if [ $? -eq 0 ]; then
                echo "✓ pip and setuptools upgraded"
            else
                echo "⚠️  Failed to upgrade pip and setuptools"
            fi

            # Install demucs in the virtual environment
            echo ""
            echo "Installing demucs in the virtual environment..."
            sudo -u "$ACTUAL_USER" "$VENV_PATH/bin/pip" install demucs

            if [ $? -eq 0 ]; then
                echo "✓ demucs installed successfully in $VENV_PATH"
                echo "Location: $VENV_PATH/bin/demucs"
                echo ""
                echo "Verifying demucs installation..."
                sudo -u "$ACTUAL_USER" "$VENV_PATH/bin/demucs" --help > /dev/null 2>&1
                if [ $? -eq 0 ]; then
                    echo "✓ demucs is working correctly"
                else
                    echo "⚠️  demucs installed but verification failed"
                fi
            else
                echo "⚠️  Failed to install demucs"
                exit 1
            fi

            # Check if MP4Box is already installed system-wide
            echo ""
            if command -v MP4Box &> /dev/null; then
                echo "✓ MP4Box is already installed at: $(command -v MP4Box)"
                MP4BOX_PATH=$(command -v MP4Box)
            else
                echo "MP4Box not found. Compiling GPAC v2.4.0 from source..."

                GPAC_BUILD_DIR="/tmp/gpac_build"

                # Create build directory
                mkdir -p "$GPAC_BUILD_DIR"
                cd "$GPAC_BUILD_DIR"

                # Download GPAC v2.4.0
                echo "Downloading GPAC v2.4.0..."
                wget -q https://github.com/gpac/gpac/archive/refs/tags/v2.4.0.tar.gz -O gpac-2.4.0.tar.gz

                if [ ! -f "gpac-2.4.0.tar.gz" ]; then
                    echo "⚠️  Failed to download GPAC"
                    exit 1
                fi

                # Extract
                echo "Extracting GPAC..."
                tar -xzf gpac-2.4.0.tar.gz
                cd gpac-2.4.0

                # Configure and compile for system installation
                echo "Configuring GPAC..."
                ./configure --prefix=/usr/local

                if [ $? -ne 0 ]; then
                    echo "⚠️  GPAC configuration failed"
                    exit 1
                fi

                echo "Compiling GPAC (this may take a few minutes)..."
                make -j$(nproc)

                if [ $? -ne 0 ]; then
                    echo "⚠️  GPAC compilation failed"
                    exit 1
                fi

                echo "Installing GPAC to system..."
                sudo make install

                if [ $? -eq 0 ]; then
                    echo "✓ GPAC v2.4.0 compiled and installed successfully"
                    echo "Location: /usr/local/bin/MP4Box"

                    # Verify MP4Box
                    if [ -f "/usr/local/bin/MP4Box" ]; then
                        echo "✓ MP4Box executable found"
                        /usr/local/bin/MP4Box -version
                        MP4BOX_PATH="/usr/local/bin/MP4Box"
                    else
                        echo "⚠️  MP4Box executable not found after installation"
                        exit 1
                    fi
                else
                    echo "⚠️  Failed to install GPAC"
                    exit 1
                fi

                # Clean up build directory
                cd /
                rm -rf "$GPAC_BUILD_DIR"
            fi

            echo ""
            echo "✓ Mixxx Python environment setup complete!"
            echo "Virtual environment location: $VENV_PATH"
            echo "Demucs: $VENV_PATH/bin/demucs"
            echo "MP4Box: $MP4BOX_PATH"
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
