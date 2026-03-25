# Compiling the library

libremidi uses CMake as build system.
The easiest way to compile the library is to take inspiration from the CI scripts as they will install the required dependencies.

Some backends require an up-to-date C++20 compiler: JACK and PipeWire, as we leverage the C++20 semaphore support.

Compiling the library and examples is as simple as:

```bash
$ cmake -Wno-dev \
  -S path/to/libremidi \
  -B build_folder \
  -DLIBREMIDI_EXAMPLES=1 

$ cmake --build build_folder
```

libremidi is also available on [vcpkg](https://vcpkg.link/ports/libremidi) and [Nixpkgs](https://mynixos.com/nixpkgs/package/libremidi).

## On Linux & BSD

Note that the ALSA and PipeWire back-end rely on timerfd and eventfd which 
may not be available on very, very, very old Linux kernels (< 3.x) or some BSD kernels.

Note that the ALSA Raw back-end also needs udev access to support hotplug for USB peripherals.

- Debian & Ubuntu packages for all the back-ends: 
```
libasound-dev
libjack-jackd2-dev
libudev-dev
libpipewire-0.3-dev
```

- ArchLinux packages for all the back-ends: 
```
alsa-lib
jack2
libpipewire
systemd-libs
```

## On Windows
Note that when targetting Windows Store, WinMM APIs are not available, only UWP ones.

Both MSVC and MSYS2 are supported (on all MSYS2 toolchains).

Note that for MIDI 2 support it is currently necessary for the end-user to install the [Windows MIDI Services](https://github.com/microsoft/MIDI/releases) on their computer. As of version 5, libremidi has been tested on Windows 10 and 11 with the DP9 NAMM Preview 4 release.

## On macOS & iOS
Note that for MIDI 2 support, the application needs to target at least macOS 11.0.
e.g. `-DCMAKE_OSX_DEPLOYMENT_TARGET=11` or more recent needs to be passed to CMake.

## Advanced features

- For MIDI 1: ability to set a fixed message size upper-bound for zero-allocation scenarios, with `-DLIBREMIDI_SLIM_MESSAGE=<NBytes>` (in CMake or directly to the compiler)
