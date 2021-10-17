# Mixxx Build Documentation

## Download Mixxx source code

First, if you do not have Git installed already,
[install Git](https://git-scm.com/downloads). Once you have Git
installed, run the following command in a terminal to download
the mixxx source code:

    git clone https://github.com/mixxxdj/mixxx.git

Then, enter the mixxx code repository:

    cd mixxx

## Install build tools

### Debian, Ubuntu, and derived distributions

On Debian, Ubuntu, and derived Linux distributions, there is a script
in the mixxx repository that will install the required packages:

    tools/debian_buildenv.sh

### Fedora

If you do not already have the [RPM Fusion](https://rpmfusion.org) free repository installed:

    sudo dnf install https://mirrors.rpmfusion.org/free/fedora/rpmfusion-free-release-$(rpm -E %fedora).noarch.rpm

The RPM Fusion nonfree repository is not required.

Then:

    sudo dnf builddep mixxx
    sudo dnf install gcc-c++ ccache ninja-build qt5-qtdeclarative-devel qt5-qtquickcontrols qt5-qtquickcontrols2

### macOS

First, install the XCode C++ toolchain:

    xcode-select --install

Then install more development tools from [Homebrew](https://brew.sh):

    brew install cmake ccache ninja nasm automake mono

Do *not* install Mixxx's dependencies from Homebrew; only install the build tools
listed above. CMake will automatically download Mixxx's dependencies with vcpkg.

Mixxx does not support ARM macOS builds yet, but the build system will build x86-64
executables from ARM macOS which run fine on ARM macOS. We will not be able to
build Mixxx for ARM macOS until we switch to Qt6.

### Windows

Using the [Visual Studio installer](https://visualstudio.microsoft.com/downloads/),
install the `Desktop development with C++` and `Python development` workloads
(Python is needed for
[pre-commit](https://github.com/mixxxdj/mixxx/wiki/Using%20Git#set-up-automatic-code-checking)).
If you have Windows set to a language other than English, you will also need
to install the English language pack from the `Language packs` tab in the
Visual Studio installer (this is
[required by vcpkg](https://github.com/microsoft/vcpkg/issues/2295)).

CMake will automatically use vcpkg to download Mixxx's dependencies.

We highly recommend to install sccache as well to speed up rebuilds. sccache requires
[installing a Rust toolchain](https://www.rust-lang.org/learn/get-started), then:

    cargo install --git https://github.com/mozilla/sccache.git --rev 3f318a8675e4c3de4f5e8ab2d086189f2ae5f5cf

## Build Mixxx

The instructions below are for building Mixxx from a command line shell. You
can use the CMake integration in the IDE of your choice instead of these
command line instructions if you prefer.

On Windows, compiling must be done from a `x64 Native Tools Command Prompt for
VS 2019` shell. Opening a regular cmd or PowerShell prompt will not work.

First, configure CMake (`-G Ninja` is required on Windows and recommended on
Linux and macOS):

    cmake -G Ninja -S . -B build

This will create a directory called `build` where the build artifacts will be.
On Windows and macOS, vcpkg will automatically download Mixxx's dependencies
during the first CMake configure step, which will take a few minutes.

To build Mixxx:

    cmake --build build

Configuring CMake only needs to be done once, or when changing a CMake option.
If you reconfigure CMake, you will not need to wait for vcpkg to redownload
Mixxx's dependencies unless you delete the `vcpkg_installed` directory inside
the CMake build directory (or delete the whole CMake build directory). After
the initial configuration, when you edit the source code, you only need to
run the build step.

If you want to switch between different CMake configurations frequently, you
can use separate build directories by replacing `build` in the steps above
with another directory.

## Run Mixxx

    build/mixxx

On Linux, if you use Wayland, to get the waveforms to show, you need to run:

    build/mixxx -platform xcb

On Linux, if you use an ALSA device for Mixxx but otherwise use PulseAudio:

    pasuspender build/mixxx

This is not necessary with PipeWire. PipeWire releases the ALSA device
so Mixxx can use it if no applications are using it through PipeWire, or
alternatively you can use PipeWire through the JACK API by selecting the JACK
backend in Mixxx's Sound Hardware preferences.

## Contributing code

If you want to contribute code to Mixxx, refer to
[Using Git](https://github.com/mixxxdj/mixxx/wiki/Using%20Git) for an overview
of our development workflow and how to set up our automatic code checking with
pre-commit.

## Command line options

For information about command line options:

    build/mixxx --help

## Install Mixxx

Installing Mixxx is not required to run it. Instead, we recommend running
Mixxx from the CMake build directory as described above. Nevertheless, if
you want to install Mixxx to the default /usr/local prefix,

    sudo cmake --install build

If you want to install Mixxx elsewhere, you can specify
`-D CMAKE_INSTALL_PREFIX=/some/path` to the CMake configure step.

If you want to uninstall Mixxx built without a package manager, you need
to keep the CMake build directory and run:

    xargs rm < build/install_manifest.txt

## Running tests

    cd build
    ctest

## macOS packaging

To create DMG app bundle installers for macOS, configure CMake with the `MACOS_BUNDLE` option:

    cmake -G Ninja -D MACOS_BUNDLE=ON -S . -B build

To create a signed package:

    cmake -G Ninja -D MACOS_BUNDLE=ON -D APPLE_CODESIGN_IDENTITY=<your signing identity> -S . -B build

After building Mixxx:

    cd build
    cpack -G DragNDrop

## Windows packaging

To create an .msi installer, you need to install the [WiX toolset](https://wixtoolset.org/releases/).
After building Mixxx:

    cd build
    cpack -G WIX
