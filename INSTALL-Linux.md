This document explains how to compile and install Mixxx from source code on Linux.

# Install build dependencies

## Debian & Derivatives (e.g. Ubuntu, Raspbian)

There is a script in the code repository that will download and install all dependencies:
```sh
tools/debian_buildenv.sh setup
```

## Fedora

On Fedora, [enable the RPMFusion package repository](http://rpmfusion.org/Configuration).
You only need to enable the *free* repository; the *nonfree* repository is not necessary for Mixxx.
Then run:
``` sh
sudo dnf groupinstall "Development Tools"
sudo dnf install gcc-c++ ccache qt5-declarative-devel fdk-aac-free
sudo dnf builddep mixxx
```

## Arch & Derivatives

First, you need to install the development tools:

    # pacman -S base-devel git cmake ccache

Mixxx dependencies can be installed from the [AUR package](https://aur.archlinux.org/packages/mixxx-git/):

    $ git clone https://aur.archlinux.org/mixxx-git.git
    $ cd mixxx-git
    $ makepkg -soe

## Nix & NixOS

### Creating a release

```
nix-build shell.nix --arg releaseMode true --arg defaultLv2Plugins true
```
This will build a fully functional mixxx derivate to run at any time.

### Development Environment

To get a working development environment start a nix-shell like this:

```
nix-shell --arg enableKeyfinder true --arg defaultLv2Plugins true
```
You can then use the commands `configure`, `build`, `run`, `debug` for your workcycle. The result will be placed in the folder cbuild.
ccache is used inside the development shell to speed up your recompile times.

# Configure

Mixxx uses the CMake build system. Building and installing Mixxx follows the standard CMake procedures.

Before compiling, you need to configure with CMake. This only needs to be done once; you don't need to repeat it when you compile Mixxx again.

This step checks if you have all the dependencies installed, similar to the configure script of GNU autotools. `/usr/local` is used as the installation path in this example, but you can set this to anywhere as long as your `$PATH` environment variable includes a `bin` directory under the installation path (`/usr/local/bin` if the installation path is `/usr/local`). Don't use the prefix /usr, because it is reserved for packaged version of Mixxx (deb/rpm/...) and will interfere with the update process of your package manager.

These examples assume you have the Mixxx source code at `~/mixxx`. If you have it elsewhere, use that path instead in the following commands.

```shell
cmake -DCMAKE_INSTALL_PREFIX=/usr/local -S ~/mixxx -B ~/mixxx/build
```

# Compile
```shell
cmake --build ~/mixxx/build --parallel `nproc`
```

# Install
```shell
cmake --build ~/mixxx/build --target install --parallel `nproc`
```

If you want to compile and install in one step, you can skip the compilation step above and just run this command.

# Run without installing
The `mixxx` binary will be in the CMake build directory. You can simply run it directly:
```shell
~/mixxx/build/mixxx
```

# ccache

We highly recommend installing [CCache](https://ccache.dev/) if you are building Mixxx frequently, whether for development or testing. CCache drastically speeds up the time to recompile Mixxx, especially when switching between git branches. CMake works with CCache automatically.

You will probably want to increase the default ccache size of 5.0GB to something much larger to accommodate Mixxx's large build sizes. You can adjust the cache size with the --set-config flag:
```sh
ccache --set-config=max_size=20.0G
```

# Development tips

## Debug build and assertions

If you want to make a debug build, add `-DCMAKE_BUILD_TYPE=Debug -DDEBUG_ASSERTIONS_FATAL=ON` to the end of the cmake configure command.
Debug builds should be started with the command line option `--debugAssertBreak` to trigger a breakpoint in the debugger if debug
assertions are violated or to abort Mixxx immediately. This ensures that messages about violated debug assertions are not missed between various other debug log messages. We recommend this if you are working on Mixxx code, but not if you are performing with Mixxx.

## Non-System Qt

To build Mixxx with a version of Qt older or newer than your distribution's package manager, download the latest [Qt source
code](https://download.qt.io/archive/qt/). For each Qt version, it is available at that link in a directory called "single" and has a filename like `qt-everywhere-src-VERSION.tar.xz`. Extract that archive and compile the source code:

```shell
tar xf qt-everywhere-src-VERSION.tar.xz
cd qt-everywhere-src-VERSION
./configure -prefix /path/to/qt/install -system-sqlite -sql-sqlite -qt-zlib -opensource -confirm-license -nomake examples -nomake tests -skip qt3d -skip qtwebengine
make -j`nproc`
make install
```

Append `-DCMAKE_PREFIX_PATH=/path/to/qt/install` (where `/path/to/qt/install` is the path you used when building Qt) to the cmake configure command to instruct cmake to prefer the Qt version from that path.
