#!/usr/bin/env sh
set -ex

# Install ninja locally
wget https://github.com/ninja-build/ninja/releases/download/v1.7.2/ninja-linux.zip && unzip -q ninja-linux.zip -d build
ls -ahl ./build
export PATH="${PWD}/build/:${PATH}"

# Install meson
sudo pip3 install meson

# Install build-time dependencies
sudo apt-get install libasound2-dev libusb-1.0-0-dev libjack-dev libsndfile-dev python3 python3-pip

# Clone and build ctlra
git clone https://github.com/openAVproductions/openAV-ctlra ctlra
cd ctlra
export CTLRA_PATH=`pwd`
echo $CTLRA_PATH
meson build
cd build
ninja
cd ..
cd ..
