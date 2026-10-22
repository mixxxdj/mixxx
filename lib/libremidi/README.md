# libremidi

[![Build status](https://github.com/jcelerier/libremidi/workflows/Build/badge.svg)](https://github.com/jcelerier/libremidi/actions)

[![Packaging status](https://repology.org/badge/vertical-allrepos/libremidi.svg?columns=3&header=libremidi)](https://repology.org/project/libremidi/versions)

libremidi is a cross-platform C++20 library for real-time and MIDI file input and output.

This is a fork / rewrite based on two libraries: 

* [RtMidi](https://github.com/theSTK/RtMidi)
* [ModernMIDI](https://github.com/ddiakopoulos/ModernMIDI)

Additionnally, for MIDI 2 parsing support we use [cmidi2](https://github.com/atsushieno/cmidi2)!

Read the documentation [here](https://celtera.github.io/libremidi).

## Citation

If you use this work as part of academic research, please kindly cite the [paper](https://smcnetwork.org/smc2024/papers/SMC2024_paper_id104.pdf): 
```bibtex
@inproceedings{celerier2024libremidi,
  title={libremidi: a cross-platform library for real-time MIDI 1 and 2},
  author={Celerier, Jean-Micha{\"e}l},
  booktitle={Proceedings of the Sound and Music Computing Conference (SMC)},
  year={2024},
  address={Porto, Portugal}
}
```

## Changelog 

### Since v5.4

* ALSA: observation will now slightly be delayed as otherwise udev fields are sometimes not populated yet.
* Port the Android backend from RtMidi, developed by @YellowLabrador.
* C++: add initial C++ modules support: `import libremidi;`. So far compilers other than Clang 20+ crash. Enable with `-DLIBREMIDI_LIBRARY_MODE=MODULE`. Note that one cannot mix a module-based and a non-module based API. An example is provided in `examples/modules.cpp`
* Add a utility `libremidi::set_client_name` / `client_name` function to simplify construction of an API object with a custom client name.
* Many improvements to MIDI 1 <-> MIDI 2 conversion, every MIDI 1 message is supported now.
* Port information: add as much metadata as we can get from the host API.
* Port information: remove sorting and comparison as there's no generally correct way to compare two `port_information` / `input_port` / `output_port` objects. Multiple ways of doing this comparison depending on the use case have been provided as examples in `<libremidi/port_comparison.hpp>`.
* Port information: added a heuristics-based `libremidi::find_closest_port(query, existing_ports)` utility function which tries to lookup the most likely candidate for a MIDI port across backends with the lookup information that can be provided.
* Python: add a pyproject.toml to facilitate integration with the Python ecosystem. Thanks @TheStaticTurtle!
* Windows MIDI Services: support updated to the [RC1 release](https://github.com/microsoft/MIDI/releases/) headers.
* Windows MIDI Services: add support for the newly introduced COM fast-path to provide maximum performance.

### Since v5.3

* Minor bugfixes
* More MinGW CI
* Add an example of converting MIDI files to .pat format
* MSVC ARM64 CI

### Since v5.2

* Minor bugfixes
* Support detecting presence of ALSA sequencer at runtime
* Add conversion functions for MIDI 1 <-> 2

### Since v5.1

* Report USB device identifiers with ALSA and udev
* PipeWire and JACK UMP support (requires PipeWire v1.4+)

### Since v5
* Use stdx::error for error reporting until C++26 and std::error are widely available :-)
* Hunt exceptions down
* MIDI 2 support on Windows with the Windows MIDI Services.
  * Using Developer Preview 9: https://github.com/microsoft/MIDI/releases/
  * Works on both MSVC and MinGW / MSYS2.
* WinUWP support on MinGW / MSYS2.
* Getters for USB location, etc. in `libremidi::port_information`.
* Reverse-engineered implementation of [Mackie Control Universal & Logic Control protocols](https://github.com/celtera/libremidi/blob/master/include/libremidi/protocols/remote_control.hpp). Tested with [TouchMCU](https://github.com/NicoG60/TouchMCU) and a BCF2000.
* C API for bindings to other languages ([libremidi-c.h](https://github.com/celtera/libremidi/blob/master/include/libremidi/libremidi-c.h)).
* [Python binding](https://github.com/celtera/libremidi/tree/master/bindings/python).
* [Haskell binding](https://github.com/ejconlon/libremidi-haskell) courtesy of @ejconlon λ!
* [Java binding](https://github.com/atsushieno/libremidi-javacpp) courtesy of @atsushieno!
* Support for getting raw, unfiltered MIDI data (e.g. no SYSEX recombination logic).
* Computer Keyboard backend to easily map scancodes to keyboard-like MIDI maps ⌨.
* [Network backend](https://github.com/celtera/libremidi/blob/master/examples/network.cpp) to send MIDI 1 and UMP packets over OSC 🛜.
* libremidi finally supports MIDI 2 on all desktop platforms 🎉!

### Since v4.5
* Input logic refactored across all backends.
  * e.g. previously not backends had the same rules wrt timestamping, sysexes, etc. Now there is a single MIDI state machine which processes this.
* PipeWire support.
* Many bugfixes across the stack.

### Since v4.4
* iOS support restored (thanks @fwcd)
* CI work: added Debian Bullseye, Bookworm, Trixie and make sure UMP code is being built.
* Add compatibility with [ni-midi2](https://github.com/midi2-dev/ni-midi2): the libremidi::ump type will convert automatically from / to midi::universal_packet and it is possible to send directly some ni-midi2 data types through libremidi::midi_out.
* Added an example of very basic MIDI-CI interoperation with MIDI2.0Workbench: https://github.com/jcelerier/libremidi/blob/master/examples/midi2_interop.cpp
* Observer: add a `track_any` flag to track MIDI ports that are not reported as being hardware or software.
* UMP: allow send_ump to handle UMP streams, not only single UMP packets.

### Since v4.3
* Improvements to timing handling.
  * Added a `Custom` timestamping mechanism which allows the user to provide a 
    custom callback to run timestamping as close as possible to the event's reception.
  * Added `midi_in::absolute_timestamp()` to get the origin timestamp for driver-provided ticks as accurately as possible.
    * e.g. in practice this is taking the time just near the ALSA queue creation or WinMM MIDI open.
  * Bugfixes in JACK
  * Many warning fixes - thanks @lilggamegenius for the extensive work!
  * Fix MIDI dump example - thanks @chdiesch!
  * Add SOVERSION to dynamic library

### Since v4.2
* More robust MIDI 2.0 support.
  * On macOS through CoreMIDI (input / output, requires macOS 11+).
  * On Linux through ALSA sequencer API (input / output, requires kernel 6.5+ and latest libasound).
  * The API can be used through MIDI 1 or MIDI 2 affordances, e. g. one can send UMP packets to a MIDI 1 API, they will get converted automatically.
  * More backends to come, in particular with the new [Windows MIDI Services](https://github.com/microsoft/MIDI) started [here](https://github.com/jcelerier/libremidi/tree/master/include/libremidi/backends/winmidi)!
  * Sysex handling on MIDI 2.0 is the responsibility of the user of the library, which simplifies the design immensely and allows the library to be used in stricter real-time contexts (as an UMP message has a fixed size, unlike MIDI 1 sysex which required potentially unbounded dynamic allocations with the previous design).
  * See [midi_echo.cpp](https://github.com/jcelerier/libremidi/blob/master/examples/midi2_echo.cpp) for a complete example.
* Linux: libasound and udev are now entirely loaded at run-time through `dlopen`. This is necessary for making apps that will run on older Linux versions which do not have the ALSA UMP APIs yet.
  Note that to make an app which supports MIDI 2 on recent Linuxes and still runs on older ones, you will need to use the latest ALSA library headers as part of your build on an older distribution, by building [alsa-lib](https://github.com/alsa-project/alsa-lib) yourself (as the old distributions with an old glibc that you want to build against to make compatible software of course also ship an old `libasound` which won't have the UMP API...).

### Since v4
* Experimental MIDI 2.0 support.
* A neat configuration system which enables to pass options to the underlying backends.
* Possibility to share the contexts across inputs and outputs to avoid creating multiple clients in e.g. JACK.
* Hotplug support for all the backends!
* Reworked port opening API which now uses handles instead of port indices to increase robustness in the face of disconnection / reconnection of MIDI devices.
* Integer timestamps everywhere, and in nanoseconds. Additionnally, it is now possible to choose different timestamping methods (e.g. relative, absolute monotonic clock...).
* Experimental API to allow to poll manually in ALSA (Sequencer and Raw), in order to give more control 
  to the user and enable processing events on any kind of Linux event-loop.
* Increase the checks done by the MIDI parser.
* Internally it's pretty much a complete rewrite. Standard threading primitives are now used, as well as modern Linux facilities for polling control (eventfd, timerfd).
  Most of the code has been refactored.
* Ability to set a fixed message size for zero-allocation scenarios, with `-DLIBREMIDI_SLIM_MESSAGE=<NBytes>` (in CMake or directly to the compiler)

### Since v3
* Allow to pass `span` when available (C++20) or `(uint8_t* bytes, std::size_t size)` pairs whenever possible to reduce copying.

### Since v1
* The library can be used header-only, [as explained in the docs](docs/header-only.md)
* Callbacks are passed by `std::function` and generally simplified.
* Ability to use `boost::small_vector` to pass midi bytes instead of `std::vector` to reduce allocations.
* Less indirections, virtuals and memory allocations.
* Simplify usage of some functions, use C++ return style everywhere.
* Use of standard C++ `snake_case`.
* Simplification of exceptions.
* Passes clean through clang-tidy, clang analyzer, GCC -Wall -Wextra, ASAN, UBSAN etc etc.
* Support chunking of output data (only supported on raw ALSA backend so far).

#### New & improved backends
* JACK support on Windows.
* JACK support through weakjack to allow runtime loading of JACK.
* UWP MIDI support on Windows
* Emscripten support to run on a web browser with WebMIDI.
* Raw ALSA support in addition to the existing ALSA sequencer support.

## Roadmap
* Migrate to std::expected instead of exceptions for error handling.
* Finish MIDI 2 implementations, provide helpers, etc.
* ~More tests and compliance checks~
* ~Work even more towards this library being a zero-cost abstraction on top of native MIDI APIs~
* ~Rethink some design issues with the original RtMidi, for instance the way port numbers work is not reliable~
* ~Refactor duplicated code across backends~

# They use this library

* [BlastThruReborn](https://github.com/Cacodemon345/BlastThruReborn)
* [BraneEngineDx](https://github.com/MC-117/BraneEngineDx)
* [chordcat](https://github.com/shriramters/chordcat)
* [EraeTouchLite](https://github.com/TheTechnobear/EraeTouchLite)
* [MIDIVisualizer](https://github.com/kosua20/MIDIVisualizer)
* [ossia.io](https://ossia.io)
* [obs-midi-mg](https://github.com/nhielost/obs-midi-mg)
* [SceneSwitcher](https://github.com/WarmUpTill/SceneSwitcher)
* [Senos](https://github.com/RuiVarela/Senos)
* [SynthMania](https://github.com/HyperLan-git/Synthmania)
* [zing](https://github.com/Rezonality/zing)
