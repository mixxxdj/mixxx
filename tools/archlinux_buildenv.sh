#!//bin/bash

set -o pipefail

sudo pacman -S --needed --noconfirm protobuf vamp-plugin-sdk \
    chromaprint libid3tag rubberband soundtouch \
    lame libogg libmad libvorbis libmp4v2 faad2 opusfile wavpack \
    libshout libsndfile portmidi portaudio \
    sqlite upower lilv libebur128 libmodplug \
    qt6-declarative qtkeychain-qt6 qt6-svg qt6-shadertools \
    qt6-5compat qt6-multimedia-ffmpeg qt6-scxml microsoft-gsl

# Checking if taglib is installed and build it from the AUR if this
# is not the case
if pacman -Qi taglib1 >/dev/null 2>&1; then
    taglib1_installed=true
else
    taglib1_installed=false
fi

if [ "$taglib1_installed" == false ]; then
    echo 'Building taglib1 from the AUR'
    rm -rf /tmp/taglib1 > /dev/null 2>&1
    git clone https://aur.archlinux.org/taglib1.git /tmp/taglib1 \
        && cd /tmp/taglib1 \
        && sed -i 's/arch=(x86_64)/arch=(x86_64 aarch64)/' PKGBUILD \
        && makepkg -si --noconfirm
fi
