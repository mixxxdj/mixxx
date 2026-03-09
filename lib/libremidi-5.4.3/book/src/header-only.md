# Using libremidi in header-only mode

The library can be used header-only, with minimal modifications to your build system if you aren't using CMake:

* Define `LIBREMIDI_HEADER_ONLY=1`
* Define macros for the APIs you wish to build. The possible macros are as follows: 

  * macOS: `LIBREMIDI_COREMIDI=1` and link against `-framework CoreMIDI -framework CoreAudio -framework CoreFoundation`.
  * Linux: `LIBREMIDI_ALSA=1` and link against `-lasound -phtread`.
  * Windows (WinMM): `LIBREMIDI_WINMM=1` and link against `winmm`.
  * Windows (UWP): `LIBREMIDI_WINUWP=1` ; note that there is complex linking logic detailed in the CMakeLists.txt when using UWP.
  * Windows (MIDI services): `LIBREMIDI_WINMIDI=1`. Windows SDK headers and Windows MIDI headers are required.
  * emscripten: `LIBREMIDI_EMSCRIPTEN=1`.
  * Any platform with JACK: `LIBREMIDI_JACK=1`.
  * Network API: `LIBREMIDI_NETWORK=1` and include boost.
  * Keyboard API: `LIBREMIDI_KEYBOARD=1`.

* Add the `include` folder to your include path.
* `#include <libremidi/libremidi.hpp>` in your source code.

For instance, to build the `midiprobe` example on Linux with only ALSA support, one would run:

    $ g++ ~/libremidi/tests/midiprobe.cpp \
          -std=c++20 \
          -DLIBREMIDI_ALSA=1 \
          -DLIBREMIDI_HEADER_ONLY=1 \
          -I ~/libremidi/include \
          -lasound -pthread

To build it on macOS, one would run:

    $ clang++ ~/libremidi/tests/midiprobe.cpp \
          -std=c++20 \
          -DLIBREMIDI_COREMIDI=1 \
          -DLIBREMIDI_HEADER_ONLY=1 \
          -I ~/libremidi/include \
          -framework CoreMIDI -framework CoreAudio -framework CoreFoundation
