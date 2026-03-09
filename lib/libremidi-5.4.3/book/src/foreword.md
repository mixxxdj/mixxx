# Funky MIDI with [libremidi](https://github.com/celtera/libremidi)

libremidi is an all-in-one cross-platform C++20 MIDI library for both file and real-time output.
Real-time I/O supports MIDI 2 on macOS (11+) and Linux (Kernel 6.5+), and on Windows (currently on Windows 11 Insiders builds or Windows 10 with an explicit install of the new [Windows MIDI Services](https://github.com/microsoft/MIDI/releases)).

It is a fork / rewrite originally based on two libraries, but has since then been almost entirely rewritten:
- [RtMidi](https://github.com/theSTK/RtMidi)
- [ModernMIDI](https://github.com/ddiakopoulos/ModernMIDI)

Compared to its origins, it features a lot of changes and improvements:

- `libremidi::observer` allows to enumerate MIDI devices and provides hotplug support on every back-end.
- Ports are identified not with a number but with a handle which enables more stability when unplugging / replugging.
- Memory allocations and virtual function calls are greatly reduced when compared to the RtMidi base-line. 
  - Ability to enforce fixed message sizes with boost::static_vector for hard real-time operation
  - Ability to use boost::small_vector to cover most cases.
- Integer timestamps everywhere, by default in nanoseconds. This avoids precision issues for instance when doing precise computations over long-running art installations mixing short and long timescales: in double-precision, the assertion "number of nanoseconds in a year + 1 == number of nanoseconds in a year" holds. 
- Ability to choose different timestamping methods (e.g. relative, absolute monotonic clock, sample-based or custom timestamping...).
- Integration of modern C++20 types (for instance std::span instead of std::vector, std::function for callbacks, etc.)
- Standard C++ threading primitives (std::thread, std::jthread) are now used, as well as modern Linux facilities for polling control (eventfd, timerfd). 
- Most of the code has been refactored in multiple files for clarity.

It also features some new & improved backends:

- ALSA RawMidi API.
- PipeWire.
- Windows UWP.
- WebMIDI in Emscripten.
- JACK support on all platforms where it is available.
- Computer keyboard input.
- Network input and output.